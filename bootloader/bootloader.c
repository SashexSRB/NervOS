#include "efi.h"

//EFI Image Entry Point
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle; //Prevent compiler warning

    // Set text to red, BG to black
    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));

    //Clear screen to bg color
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

    // Print Hello World
    SystemTable->ConOut->OutputString(SystemTable->ConOut, u"Welcome to NervOS!\r\n\r\n");
    
    // Set text to red, BG to black
    SystemTable->ConOut->OutputString(SystemTable->ConOut, EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));

    SystemTable->ConOut->OutputString(SystemTable->ConOut, u"Press any key to shutdown...");

    // Wait until keypress, then return
    EFI_INPUT_KEY key;
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) != EFI_SUCCESS);

    // Shutdown, does not return
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

    // Should never get here
    return EFI_SUCCESS;
}