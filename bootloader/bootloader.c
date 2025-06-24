#include "efi.h"
#include <stdbool.h>
#include <stdarg.h>
// -----------------
//  Global macros
// -----------------
#define ARR_SIZE(x) (sizeof (x) / sizeof (x)[0])

// -----------------
//  Global constants
// -----------------
#define DEFAULT_FG_COLOR EFI_RED
#define DEFAULT_BG_COLOR EFI_BLACK
#define HL_FG_COLOR EFI_BLACK
#define HL_BG_COLOR EFI_LIGHTGRAY

// -----------------
//  Global vars
// -----------------
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout = NULL; // Console out
EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL; // Console out
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cerr = NULL; // Console error
EFI_BOOT_SERVICES *bs; // Boot Services
EFI_RUNTIME_SERVICES *rs; // Runtime Services
EFI_HANDLE image = NULL; // Image handle

// =================
// Set global vars
// =================
void initGlbVars(EFI_HANDLE handle, EFI_SYSTEM_TABLE *systable) {
    cout = systable->ConOut;
    cin = systable->ConIn;
    //cerr = systable->StdErr; // TODO: Research why it doesnt work in emulation
    cerr = cout; // Temporary
    bs = systable->BootServices;
    rs = systable->RuntimeServices;
    image = handle;
}

// =================
// Print an number to stderr
// =================
BOOLEAN eprintNum(UINTN number, UINT8 base, BOOLEAN isSigned) {
    const CHAR16 *digits = u"0123456789ABCDEF";
    CHAR16 buffer[24]; // enough for UINTN_MAX (UINT64_MAX) + sign
    UINTN i = 0;
    BOOLEAN negative = FALSE;

    if(base > 16) {
        cerr->OutputString(cerr, u"Invalid Base Specified!\r\n");
        return FALSE;
    } // Invalid base

    // only use and print neg numbers if decimal is signed. 
    if(base == 10 && isSigned && (INTN)number < 0) {
        number = -(INTN)number; // Get absolute value of correct signed value to get digits to print
        negative = TRUE;
    }

    do {
        buffer[i++] = digits[number % base];
        number /= base;
    } while(number > 0);

    switch(base) {
        case 2:
            // binary
            buffer[i++] = u'b';
            buffer[i++] = u'0';
            break;
        case 8:
            // octal
            buffer[i++] = u'o';
            buffer[i++] = u'0';
            break;
        case 10: 
            // decimal
            if (negative) buffer[i++] = u'-';
            break;
        case 16:
            // hexadecimal
            buffer[i++] = u'x';
            buffer[i++] = u'0';
            break;
        default:
            // Maybe invalid base, but we'll go with it (no processing)
            break;
    }
    
    // null terminate string first
    buffer[i--] = u'\0';

    // reverse buffer before printing
    for (UINTN j = 0; j < i; j++, i--) {
        // swap digits
        UINTN temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    // print number string
    cerr->OutputString(cerr, buffer);

    return TRUE;
}

// =================
// Print an number to stdout
// =================
BOOLEAN printNum(UINTN number, UINT8 base, BOOLEAN isSigned) {
    const CHAR16 *digits = u"0123456789ABCDEF";
    CHAR16 buffer[24]; // enough for UINTN_MAX (UINT64_MAX) + sign
    UINTN i = 0;
    BOOLEAN negative = FALSE;

    if(base > 16) {
        cerr->OutputString(cerr, u"Invalid Base Specified!\r\n");
        return FALSE;
    } // Invalid base

    // only use and print neg numbers if decimal is signed. 
    if(base == 10 && isSigned && (INTN)number < 0) {
        number = -(INTN)number; // Get absolute value of correct signed value to get digits to print
        negative = TRUE;
    }

    do {
        buffer[i++] = digits[number % base];
        number /= base;
    } while(number > 0);

    switch(base) {
        case 2:
            // binary
            buffer[i++] = u'b';
            buffer[i++] = u'0';
            break;
        case 8:
            // octal
            buffer[i++] = u'o';
            buffer[i++] = u'0';
            break;
        case 10: 
            // decimal
            if (negative) buffer[i++] = u'-';
            break;
        case 16:
            // hexadecimal
            buffer[i++] = u'x';
            buffer[i++] = u'0';
            break;
        default:
            // Maybe invalid base, but we'll go with it (no processing)
            break;
    }
    
    // null terminate string first
    buffer[i--] = u'\0';

    // reverse buffer before printing
    for (UINTN j = 0; j < i; j++, i--) {
        // swap digits
        UINTN temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    // print number string
    cout->OutputString(cout, buffer);

    return TRUE;
}

// =================
// Print formatted strings to stderr
// =================
BOOLEAN eprintf(CHAR16 *fmt, ...) {
    BOOLEAN result = TRUE;
    CHAR16 cstr[2]; // place init this with memset and use = {} initializer
    va_list args;
    va_start(args, fmt);
    
    //Initialize buffer
    cstr[0] = u'\0', cstr[1] = u'\0';

    // Print formatted string values
    for (UINTN i=0 ; fmt[i] != u'\0'; i++) {
        if (fmt[i] == u'%') {
            i++;
            // Grab next arg type from input args and print it
            switch(fmt[i]) {
                case u's': {
                    //printf("%s", string) - Print CHAR16 string
                    CHAR16 *string = va_arg(args, CHAR16 *);
                    cerr->OutputString(cerr, string);
                }
                break;
                case u'd': {
                    //printf("%d", number_int32) - Print INT32
                    INT32 number = va_arg(args, INT32);
                    eprintNum(number, 10, TRUE);
                }
                break;
                case u'x': {
                    //printf("%d", number_int32) - Print INT32
                    UINTN number = va_arg(args, UINTN);
                    eprintNum(number, 16, FALSE);
                }
                case u'u': {
                    UINT32 number = va_arg(args, UINT32);
                    eprintNum(number, 10, FALSE);
                }
                break;
                default:
                    cerr->OutputString(cerr, u"Invalid format specifier: %");
                    cstr[0] = fmt[i];
                    cerr->OutputString(cerr, cstr);
                    cerr->OutputString(cerr, u"\r\n");
                    result = FALSE;
                    goto end;
                break;
            }
        } else {
            // Not formatted, print next char
            cstr[0] = fmt[i];
            cerr->OutputString(cerr, cstr);
        }
    }
end:
    va_end(args);
    return result;
} 

// =================
// Print formatted strings to stdout
// =================
BOOLEAN printf(CHAR16 *fmt, ...) {
    BOOLEAN result = TRUE;
    CHAR16 cstr[2]; // place init this with memset and use = {} initializer
    va_list args;
    va_start(args, fmt);
    
    //Initialize buffer
    cstr[0] = u'\0', cstr[1] = u'\0';

    // Print formatted string values
    for (UINTN i=0 ; fmt[i] != u'\0'; i++) {
        if (fmt[i] == u'%') {
            i++;
            // Grab next arg type from input args and print it
            switch(fmt[i]) {
                case u's': {
                    //printf("%s", string) - Print CHAR16 string
                    CHAR16 *string = va_arg(args, CHAR16 *);
                    cout->OutputString(cout, string);
                }
                break;
                case u'd': {
                    INT32 number = va_arg(args, INT32);
                    printNum(number, 10, TRUE);
                }
                break;
                case u'x': {
                    UINTN number = va_arg(args, UINTN);
                    printNum(number, 16, FALSE);
                }
                break;
                case u'u': {
                    UINT32 number = va_arg(args, UINT32);
                    printNum(number, 10, FALSE);
                }
                break;
                default:
                    cout->OutputString(cout, u"Invalid format specifier: %");
                    cstr[0] = fmt[i];
                    cout->OutputString(cout, cstr);
                    cout->OutputString(cout, u"\r\n");
                    result = FALSE;
                    goto end;
                break;
            }
        } else {
            // Not formatted, print next char
            cstr[0] = fmt[i];
            cout->OutputString(cout, cstr);
        }
    }
end:
    va_end(args);
    return result;
} 

// =================
// Get key from user
// =================
EFI_INPUT_KEY getKey(void) {
    EFI_EVENT events[1];
    EFI_INPUT_KEY key;
    key.ScanCode = 0;
    key.UnicodeChar = u'\0';

    events[0] = cin->WaitForKey;
    UINTN idx = 0;
    bs->WaitForEvent(1, events, &idx);

    if(idx == 0) cin->ReadKeyStroke(cin, &key);
    return key;
}

// =================
// Set text mode
// =================
EFI_STATUS setTextMode(void) {
    // Overall Screen Loop
    while(true) {
        cout->ClearScreen(cout);

        cout->OutputString(cout, u"Text mode information:\r\n");

        UINTN maxCols=0, maxRows=0;

        //Get current text mode's current cols and rows
        cout->QueryMode(cout, cout->Mode->Mode, &maxCols, &maxRows);
        printf(u"Max Mode: %d\r\n"
            u"Current Mode: %d\r\n"
            u"Attribute: %x\r\n"
            u"CursorColumn: %d\r\n"
            u"CursorRow: %d\r\n"
            u"CursorVisible: %d\r\n"
            u"Columns: %d\r\n"
            u"Rows: %d\r\n\r\n",
            cout->Mode->MaxMode,
            cout->Mode->Mode,
            cout->Mode->Attribute,
            cout->Mode->CursorColumn,
            cout->Mode->CursorRow,
            cout->Mode->CursorVisible,
            maxCols,
            maxRows
        );
        
        cout->OutputString(cout, u"Available text modes:\r\n\r\n");

        // Print other text infos
        const INT32 max = cout->Mode->MaxMode;
        for (INT32 i = 0; i < max; i++) {
            cout->QueryMode(cout, i, &maxCols, &maxRows);
            printf(u"Mode #: %d, %dx%d\r\n", i, maxCols, maxRows);
        }

        printf(u"\r\n");

        // Get number from user
        while(1) {
            static UINTN currentMode = 0;
            currentMode = cout->Mode->Mode;

            for(UINTN i = 0; i < 79; i++) printf(u" ");
            printf(u"\rSelect Text Mode (0-%d): %d", max, currentMode);
            
            // Move cursor left by 1, to overwrite the mode #
            cout->SetCursorPosition(cout, cout->Mode->CursorColumn-1, cout->Mode->CursorRow);

            EFI_INPUT_KEY key = getKey();

            // Get key info
            CHAR16 cbuf[2];
            cbuf[0] = key.UnicodeChar;
            cbuf[1] = u'\0';
            // printf(u"Scancode %x, Unicode Char: %s\r", key.ScanCode, cbuf);
            
            // Process keystroke
            printf(u"%s ", cbuf);

            if(key.ScanCode == SCANCODE_ESC) {
                // Back to Main Menu
                return EFI_SUCCESS;
            }

            // Choose text mode & redraw
            currentMode = key.UnicodeChar - u'0';
            EFI_STATUS status = cout->SetMode(cout, currentMode);
            if (EFI_ERROR(status)) {
                // Handle error
                if (status == EFI_DEVICE_ERROR) {
                    eprintf(u"ERROR: %x; DEVICE ERROR", status);
                } else if (status == EFI_UNSUPPORTED) {
                    eprintf(u"ERROR: %x; Invalid Mode", status);
                }
                printf(u"\r\nPress any key to select again", status);
                getKey();
                
            }
            // Set new mode, redraw screen from outer loop. 
            break;
        }
    }
    return EFI_SUCCESS;

}

// =================
// Set graphics mode
// =================
EFI_STATUS setGraphicsMode(void) {
    // Get GOP Protocol via LocateProtocol()
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *modeInfo = NULL;
    UINTN modeInfoSize = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    EFI_STATUS status = 0;
    bs->LocateProtocol(&gopGuid, NULL, (VOID **)&gop);

    if(EFI_ERROR(status)) {
        eprintf(u"ERROR: %x Could not locate GOP! :(\r\n", status);
        return status;
    }

    // Overall Screen Loop
    while(true) {
        cout->ClearScreen(cout);

        printf(u"Graphics mode information:\r\n");

        //Get current GOP mode information
        status = gop->QueryMode(gop, gop->Mode->Mode, &modeInfoSize, &modeInfo);

        if(EFI_ERROR(status)) {
            eprintf(u"ERROR: %x Could not Query GOP Mode %u\r\n", status, gop->Mode->Mode);
            //return status;
        }

        // Print Info
        printf(u"Max Mode: %d\r\n"
            u"Current Mode: %d\r\n"
            u"WidthXHeight: %ux%u\r\n"
            u"Framebuffer address: %x\r\n"
            u"Framebuffer size: %u\r\n"
            u"PixelFormat: %d\r\n"
            u"PixelsPerScanLine: %d\r\n",
            gop->Mode->MaxMode,
            gop->Mode->Mode,
            modeInfo->HorizontalResolution, modeInfo->VerticalResolution,
            gop->Mode->FrameBufferBase,
            gop->Mode->FrameBufferSize,
            modeInfo->PixelFormat,
            modeInfo->PixelsPerScanLine
        );
        
        cout->OutputString(cout, u"\r\nAvailable GOP modes:\r\n\r\n");

        // Print other text infos
        const UINT32 max = gop->Mode->MaxMode;
        for (UINT32 i = 0; i < max; i++) {
            gop->QueryMode(gop, i, &modeInfoSize, &modeInfo);
            printf(u"Mode #: %d, %dx%d\r\n", 
                   i, modeInfo->HorizontalResolution, modeInfo->VerticalResolution);
        }
        
        while(1);

        // TODO: Get input from user
        while(1) {
            static UINTN currentMode = 0;
            currentMode = cout->Mode->Mode;

            EFI_INPUT_KEY key = getKey();

            // Get key info
            CHAR16 cbuf[2];
            cbuf[0] = key.UnicodeChar;
            cbuf[1] = u'\0';
            // printf(u"Scancode %x, Unicode Char: %s\r", key.ScanCode, cbuf);
            
            // Process keystroke
            printf(u"%s ", cbuf);

            if(key.ScanCode == SCANCODE_ESC) {
                // Back to Main Menu
                return EFI_SUCCESS;
            }

            // Choose text mode & redraw
            // currentMode = key.UnicodeChar - u'0';
            // EFI_STATUS status = cout->SetMode(cout, currentMode);
            // if (EFI_ERROR(status)) {
            //     // Handle error
            //     if (status == EFI_DEVICE_ERROR) {
            //         eprintf(u"ERROR: %x; DEVICE ERROR", status);
            //     } else if (status == EFI_UNSUPPORTED) {
            //         eprintf(u"ERROR: %x; Invalid Mode", status);
            //     }
            //     printf(u"\r\nPress any key to select again", status);
            //     getKey();
                
            // }
            // Set new mode, redraw screen from outer loop. 
            break;
        }
    }
    return EFI_SUCCESS;
}

// =================
// EFI Image Entry Point
// =================
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Global vars init
    initGlbVars(ImageHandle, SystemTable);
    
    // Reset console output
    cout->Reset(cout, false);
    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));

    // Screen loop
    bool running = true;
    while(running) {
        const CHAR16 *menuChoices[] = {
            u"Set Text Mode",
            u"Set Graphics Mode",
        };

        const EFI_STATUS (*menuFuncs[])(void) = {
            setTextMode,
            setGraphicsMode,
        };

        cout->ClearScreen(cout);
        // Get current text mode ColsxRows values
        UINTN cols = 0, rows = 0;
        cout->QueryMode(cout, cout->Mode->Mode, &cols, &rows);

        // Print keybinds at the bottom of the screen
        cout->SetCursorPosition(cout, 0, rows-3);
        printf(u"Up/Down Arrow = Move\r\n"
               u"Enter = Select\r\n"
               u"Escape = Shutdown");

        // Print menu choices
        // Highlight first option as initial choice

        cout->SetCursorPosition(cout, 0, 0);
        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
        printf(u"%s", menuChoices[0]);

        // Print rest of options
        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
        for (UINTN i = 1; i < ARR_SIZE(menuChoices); i++) {
            printf(u"\r\n%s", menuChoices[i]);
        } 
        
        // get cursor row boundaries
        INTN minRow = 0, maxRow = cout->Mode->CursorRow;

        // Input Loop
        cout->SetCursorPosition(cout, 0, 0);
        bool gettingInput = true;
        while(gettingInput) {
            INTN currentRow = cout->Mode->CursorRow;
            EFI_INPUT_KEY key = getKey();

            // Process input
            switch(key.ScanCode) {
                case SCANCODE_UP_ARROW:
                    if(currentRow-1 >= minRow) {
                        // de-highlight current row,
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                        printf(u"%s\r", menuChoices[currentRow]);

                        currentRow--;
                        cout->SetCursorPosition(cout, 0, currentRow);
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        printf(u"%s\r", menuChoices[currentRow]);
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                    }
                break;
                case SCANCODE_DOWN_ARROW:
                    if(currentRow+1 <= maxRow) {
                        // de-highlight current row,
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                        printf(u"%s\r", menuChoices[currentRow]);

                        currentRow++;
                        cout->SetCursorPosition(cout, 0, currentRow);
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        printf(u"%s\r", menuChoices[currentRow]);
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                    }
                break;
                case SCANCODE_ESC:
                    // Escape, poweroff
                    rs->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
                    // !NOTE!: This should not return, sys should poweroff.
                break;
                default:
                    if(key.UnicodeChar == u'\r') {
                        EFI_STATUS returnStatus = menuFuncs[currentRow]();
                        if(EFI_ERROR(returnStatus)) {
                            eprintf(u"ERROR: %x\r\n; Press any key to go back...", returnStatus);
                            getKey();
                        }
                        gettingInput = false; // will leave input loop and reprint main menu
                    }
                break;
            }
        }
    }

    // End
    return EFI_SUCCESS;
}