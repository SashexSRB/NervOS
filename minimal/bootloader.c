#include "efi.h"
// -----------------
//  Global vars
// -----------------
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout = NULL; // Console out
EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL; // Console out
//void *bs // Boot Services
//void *rs // Runtime Services

// =================
// Set global vars
// =================
void initGlbVars(EFI_SYSTEM_TABLE *systable) {
    cout = systable->ConOut;
    cin = systable->ConIn;
}

// =================
// EFI Image Entry Point
// =================
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle, (void)SystemTable; //Prevent compiler warning

    // Global vars init
    initGlbVars(SystemTable);
    
    // Clear screen
    cout->ClearScreen(SystemTable->ConOut);

    // Print Hello World
    cout->OutputString(SystemTable->ConOut, u"Welcome to NervOS!\r\n\r\n");

    cout->OutputString(SystemTable->ConOut, u"Current text mode:\r\n");
    
    cout->OutputString(SystemTable->ConOut, u"Available text modes:\r\n");

    // QueryMode

    // SetMode


    // Wait until keypress, then return
    //EFI_INPUT_KEY key;
    //while (cin->ReadKeyStroke(SystemTable->ConIn, &key) != EFI_SUCCESS);

    // Shutdown, does not return
    //SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    while(1);

    // Should never get here
    return EFI_SUCCESS;
}