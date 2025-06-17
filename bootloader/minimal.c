#include <efi.h>
#include <efilib.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print(L"Minimal UEFI App Started!\n");
    SystemTable->BootServices->Stall(5 * 1000 * 1000); // 5 seconds
    return EFI_SUCCESS;
}