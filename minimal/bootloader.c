#include "efi.h"

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout = NULL; // Console out
EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL; // Console out

void initGlbVars(EFI_SYSTEM_TABLE *systable) {
    cout = systable->ConOut;
    cin = systable->ConIn;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle, (void)SystemTable; //Prevent compiler warning

    // Global vars init
    initGlbVars(SystemTable);
    
    // Clear screen
    cout->ClearScreen(cout);

    // Print Hello World
    cout->OutputString(cout, u"Hello World!\r\n\r\n");


    // Wait until keypress, then return
    EFI_INPUT_KEY key;
    while (cin->ReadKeyStroke(cin, &key) != EFI_SUCCESS);

    // Shutdown, does not return
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    while(1);

    // Should never get here
    return EFI_SUCCESS;
}