#include <efi.h>
#include <efilib.h>

// Simple ELF header structure
typedef struct {
    unsigned char e_ident[16];
    UINT16 e_type;
    UINT16 e_machine;
    UINT32 e_version;
    UINT64 e_entry; // Entry point address
    UINT64 e_phoff; // Program header offset
    UINT64 e_shoff;
    UINT32 e_flags;
    UINT16 e_ehsize;
    UINT16 e_phentsize;
    UINT16 e_phnum;
    UINT16 e_shentsize;
    UINT16 e_shnum;
    UINT16 e_shstrndx;
} Elf64_Ehdr;

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    Print(L"UEFI Bootloader Stage 1 Loaded!\n");
    Print(L"Press any key to continue...\n");
    EFI_INPUT_KEY Key;
    SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) != EFI_SUCCESS);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_FILE_HANDLE RootFS, KernelFile;
    EFI_LOADED_IMAGE *LoadedImage;
    EFI_STATUS status;

    // Get loaded image protocol
    status = SystemTable->BootServices->HandleProtocol(
        ImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&LoadedImage);
    if (EFI_ERROR(status)) {
        Print(L"Failed to get LoadedImage protocol: %r\n", status);
        return status;
    }

    // Locate handles supporting EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
    UINTN HandleCount = 0;
    EFI_HANDLE *HandleBuffer = NULL;
    status = SystemTable->BootServices->LocateHandleBuffer(
        ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to locate FileSystem protocol: %r\n", status);
        return status;
    }

    // Find the first handle with the protocol
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

    status = RootFS->Open(RootFS, &KernelFile, L"kernel.elf", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        Print(L"Failed to load kernel.elf: %r\n", status);
        return status;
    }

    // Get kernel file size
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO) + 200; // Extra space for FileName
    status = SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate pool for FileInfo: %r\n", status);
        return status;
    }
    status = KernelFile->GetInfo(KernelFile, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if (EFI_ERROR(status)) {
        Print(L"Failed to get kernel file info: %r\n", status);
        SystemTable->BootServices->FreePool(FileInfo);
        return status;
    }

    UINTN kernelSize = FileInfo->FileSize;
    VOID *kernelBuffer;

    // Allocate memory for kernel
    status = SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData,
        (kernelSize + 0xFFF) / 0x1000, (EFI_PHYSICAL_ADDRESS*)&kernelBuffer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate pages for kernel: %r\n", status);
        SystemTable->BootServices->FreePool(FileInfo);
        return status;
    }
    status = KernelFile->Read(KernelFile, &kernelSize, kernelBuffer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to read kernel: %r\n", status);
        SystemTable->BootServices->FreePool(FileInfo);
        return status;
    }
    SystemTable->BootServices->FreePool(FileInfo); // Free FileInfo after use
    Print(L"Kernel loaded at address: %lx\n", kernelBuffer);

    // Parse ELF header to get entry point
    Elf64_Ehdr *elfHeader = (Elf64_Ehdr *)kernelBuffer;
    if (elfHeader->e_ident[0] != 0x7F || elfHeader->e_ident[1] != 'E' ||
        elfHeader->e_ident[2] != 'L' || elfHeader->e_ident[3] != 'F') {
        Print(L"Invalid ELF header\n");
        return EFI_INVALID_PARAMETER;
    }
    UINT64 kernelEntry = elfHeader->e_entry;

    // Get memory map
    UINTN mapSize = 0, mapKey, descSize;
    UINT32 descVer;
    EFI_MEMORY_DESCRIPTOR *memMap = NULL;
    SystemTable->BootServices->GetMemoryMap(&mapSize, NULL, &mapKey, &descSize, &descVer);
    mapSize += descSize * 10;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, mapSize, (void**)&memMap);
    status = SystemTable->BootServices->GetMemoryMap(&mapSize, memMap, &mapKey, &descSize, &descVer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to get memory map: %r\n", status);
        return status;
    }

    // Exit boot services
    status = SystemTable->BootServices->ExitBootServices(ImageHandle, mapKey);
    if (EFI_ERROR(status)) {
        Print(L"Failed to exit boot services: %r\n", status);
        return status;
    }

    // Jump to kernel
    void (*kernel_entry)(void) = ((__attribute__((sysv_abi)) void (*)(void))kernelEntry);
    kernel_entry();

    return EFI_SUCCESS;
}