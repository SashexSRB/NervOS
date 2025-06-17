#include <efi.h>
#include <efilib.h>

typedef struct {
    unsigned char e_ident[16];
    UINT16 e_type;
    UINT16 e_machine;
    UINT32 e_version;
    UINT64 e_entry;
    UINT64 e_phoff;
    UINT64 e_shoff;
    UINT32 e_flags;
    UINT16 e_ehsize;
    UINT16 e_phentsize;
    UINT16 e_phnum;
    UINT16 e_shentsize;
    UINT16 e_shnum;
    UINT16 e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    UINT32 p_type;
    UINT32 p_flags;
    UINT64 p_offset;
    UINT64 p_vaddr;
    UINT64 p_paddr;
    UINT64 p_filesz;
    UINT64 p_memsz;
    UINT64 p_align;
} Elf64_Phdr;

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    Print(L"UEFI Bootloader Started!\n");
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_FILE_HANDLE RootFS, KernelFile;
    EFI_LOADED_IMAGE *LoadedImage;
    EFI_STATUS status;

    // Get LoadedImage protocol
    status = SystemTable->BootServices->HandleProtocol(
        ImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&LoadedImage);
    if (EFI_ERROR(status)) {
        Print(L"Failed to get LoadedImage: %r\n", status);
        return status;
    }

    // Locate FileSystem protocol
    UINTN HandleCount = 0;
    EFI_HANDLE *HandleBuffer = NULL;
    status = SystemTable->BootServices->LocateHandleBuffer(
        ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to locate FileSystem: %r\n", status);
        return status;
    }

    for (UINTN i = 0; i < HandleCount; i++) {
        status = SystemTable->BootServices->HandleProtocol(
            HandleBuffer[i], &gEfiSimpleFileSystemProtocolGuid, (void **)&FileSystem);
        if (!EFI_ERROR(status)) {
            break;
        }
    }
    SystemTable->BootServices->FreePool(HandleBuffer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to get FileSystem protocol: %r\n", status);
        return status;
    }

    status = FileSystem->OpenVolume(FileSystem, &RootFS);
    if (EFI_ERROR(status)) {
        Print(L"Failed to open volume: %r\n", status);
        return status;
    }

    status = RootFS->Open(RootFS, &KernelFile, L"kernel", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        Print(L"Failed to load kernel.elf: %r\n", status);
        RootFS->Close(RootFS);
        return status;
    }

    // Get kernel file size
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO) + 200;
    status = SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate pool for FileInfo: %r\n", status);
        KernelFile->Close(KernelFile);
        RootFS->Close(RootFS);
        return status;
    }
    status = KernelFile->GetInfo(KernelFile, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if (EFI_ERROR(status)) {
        Print(L"Failed to get kernel file info: %r\n", status);
        KernelFile->Close(KernelFile);
        RootFS->Close(RootFS);
        SystemTable->BootServices->FreePool(FileInfo);
        return status;
    }

    UINTN kernelSize = FileInfo->FileSize;
    VOID *kernelBuffer;
    status = SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData,
        (kernelSize + 0xFFF) / 0x1000, (EFI_PHYSICAL_ADDRESS*)&kernelBuffer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate kernel memory: %r\n", status);
        KernelFile->Close(KernelFile);
        RootFS->Close(RootFS);
        SystemTable->BootServices->FreePool(FileInfo);
        return status;
    }
    status = KernelFile->Read(KernelFile, &kernelSize, kernelBuffer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to read kernel: %r\n", status);
        KernelFile->Close(KernelFile);
        RootFS->Close(RootFS);
        SystemTable->BootServices->FreePool(FileInfo);
        SystemTable->BootServices->FreePages((EFI_PHYSICAL_ADDRESS)kernelBuffer, (kernelSize + 0xFFF) / 0x1000);
        return status;
    }
    KernelFile->Close(KernelFile);
    SystemTable->BootServices->FreePool(FileInfo);
    Print(L"Kernel loaded at %p\n", kernelBuffer);

    // Validate and parse ELF
    Elf64_Ehdr *elfHeader = (Elf64_Ehdr *)kernelBuffer;
    if (elfHeader->e_ident[0] != 0x7F || elfHeader->e_ident[1] != 'E' ||
        elfHeader->e_ident[2] != 'L' || elfHeader->e_ident[3] != 'F') {
        Print(L"Invalid ELF header\n");
        RootFS->Close(RootFS);
        SystemTable->BootServices->FreePages((EFI_PHYSICAL_ADDRESS)kernelBuffer, (kernelSize + 0xFFF) / 0x1000);
        return EFI_INVALID_PARAMETER;
    }
    Elf64_Phdr *phdrs = (Elf64_Phdr *)((UINT8 *)kernelBuffer + elfHeader->e_phoff);
    for (UINT16 i = 0; i < elfHeader->e_phnum; i++) {
        if (phdrs[i].p_type == 1) { // PT_LOAD
            VOID *addr = (VOID *)phdrs[i].p_paddr;
            SystemTable->BootServices->CopyMem(addr, (UINT8 *)kernelBuffer + phdrs[i].p_offset, phdrs[i].p_filesz);
            if (phdrs[i].p_memsz > phdrs[i].p_filesz) {
                SystemTable->BootServices->SetMem((UINT8 *)addr + phdrs[i].p_filesz, phdrs[i].p_memsz - phdrs[i].p_filesz, 0);
            }
        }
    }
    UINT64 kernelEntry = elfHeader->e_entry;

    // Get memory map
    UINTN mapSize = 0, mapKey, descSize;
    UINT32 descVer;
    EFI_MEMORY_DESCRIPTOR *memMap = NULL;
    SystemTable->BootServices->GetMemoryMap(&mapSize, NULL, &mapKey, &descSize, &descVer);
    mapSize += descSize * 2;
    status = SystemTable->BootServices->AllocatePool(EfiLoaderData, mapSize, (void**)&memMap);
    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate memory map: %r\n", status);
        RootFS->Close(RootFS);
        SystemTable->BootServices->FreePages((EFI_PHYSICAL_ADDRESS)kernelBuffer, (kernelSize + 0xFFF) / 0x1000);
        return status;
    }
    status = SystemTable->BootServices->GetMemoryMap(&mapSize, memMap, &mapKey, &descSize, &descVer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to get memory map: %r\n", status);
        RootFS->Close(RootFS);
        SystemTable->BootServices->FreePool(memMap);
        SystemTable->BootServices->FreePages((EFI_PHYSICAL_ADDRESS)kernelBuffer, (kernelSize + 0xFFF) / 0x1000);
        return status;
    }

    RootFS->Close(RootFS);

    // Exit boot services
    Print(L"Jumping to kernel at %p\n", kernelEntry);
    status = SystemTable->BootServices->ExitBootServices(ImageHandle, mapKey);
    if (EFI_ERROR(status)) {
        Print(L"Failed to exit boot services: %r\n", status);
        SystemTable->BootServices->FreePool(memMap);
        return status;
    }

    // Jump to kernel (adjust based on kernel requirements)
    void (*kernel_entry)(void) = (void *)kernelEntry;
    kernel_entry();

    return EFI_SUCCESS;
}