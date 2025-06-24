#pragma once
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
    typedef struct {
        INTN mode;
        UINTN rows;
        UINTN cols;
    } TextModeInfo; 
    TextModeInfo textModes[20];

    UINTN modeIdx = 0;

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

        UINTN menuTop = cout->Mode->CursorRow, menuBottom = maxRows;

        // Print keybinds at the bottom of the screen
        cout->SetCursorPosition(cout, 0, menuBottom-3);
        printf(u"Up/Down Arrow = Move\r\n"
               u"Enter = Select\r\n"
               u"Escape = Back");

        cout->SetCursorPosition(cout, 0, menuTop);
        menuBottom -= 5;
        UINTN menuLength = menuBottom - menuTop;

        // Print other text infos
        UINT32 max = cout->Mode->MaxMode;
        if(max < menuLength) menuBottom = menuTop + max-1;

        for (UINT32 i = 0; i < ARR_SIZE(textModes) && i < max; i++) {
            cout->QueryMode(cout, i, &textModes[i].cols, &textModes[i].rows);
            UINT32 j = i;
            while ((!textModes[j].cols || !textModes[j].rows) || (textModes[j].cols > 999 || textModes[j].rows > 999)) {
                cout->QueryMode(cout, ++i, &textModes[j].cols, &textModes[j].rows);
                menuBottom--;
                max--;
            }
            textModes[j].mode = i;
        }
        menuLength = menuBottom - menuTop; // Limit number of menu modes

        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
        printf(u"Mode %d: %dx%d", textModes[0].mode, textModes[0].cols, textModes[0].rows);

        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
        for(UINT32 i = 1; i < menuLength + 1; i++) {
            printf(u"\r\nMode %d: %dx%d", textModes[i].mode, textModes[i].cols, textModes[i].rows);
        }

        // Get input from user
        cout->SetCursorPosition(cout, 0, menuTop);
        bool gettingInput = true;
        while(gettingInput) {
            UINTN currentRow = cout->Mode->CursorRow;
            EFI_INPUT_KEY key = getKey();

            switch(key.ScanCode) { // Scrolling Logic
                case SCANCODE_ESC: return EFI_SUCCESS;
                case SCANCODE_UP_ARROW: 
                    if(currentRow == menuTop && modeIdx > 0) {
                        // Scroll menu up by decrementing all modes by 1
                        printf(u"                    \r");
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        modeIdx--;
                        printf(u"Mode %d: %dx%d", textModes[modeIdx].mode, textModes[modeIdx].cols, textModes[modeIdx].rows);

                        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                        UINTN tempMode = modeIdx + 1;
                        for(UINT32 i=0; i < menuLength; i++, tempMode++) {
                            printf(u"\r\n                    \r"
                                   u"Mode %d: %dx%d\r", textModes[tempMode].mode, textModes[tempMode].cols, textModes[tempMode].rows);
                        }
                        cout->SetCursorPosition(cout, 0, menuTop);
                    } else if (currentRow-1 >= menuTop){
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r", textModes[modeIdx].mode, textModes[modeIdx].cols, textModes[modeIdx].rows);
                        modeIdx--;
                        currentRow--;
                        cout->SetCursorPosition(cout, 0, currentRow);
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r", textModes[modeIdx].mode, textModes[modeIdx].cols, textModes[modeIdx].rows);
                    }
                    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                break;
                case SCANCODE_DOWN_ARROW:
                    if(currentRow == menuBottom && modeIdx < max-1) {
                        modeIdx -= menuLength -1;
                        cout->SetCursorPosition(cout, 0, menuTop);
                        for(UINT32 i = 0; i < menuLength; i++, modeIdx++) {
                            printf(u"                    \r"
                                   u"Mode %d: %dx%d\r\n", textModes[modeIdx].mode, textModes[modeIdx].cols, textModes[modeIdx].rows);
                        }
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r", textModes[modeIdx].mode, textModes[modeIdx].cols, textModes[modeIdx].rows);
                    } else if(currentRow+1 <= menuBottom) {
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r\n", textModes[modeIdx].mode, textModes[modeIdx].cols, textModes[modeIdx].rows);
                        modeIdx++;
                        currentRow++;
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r\n", textModes[modeIdx].mode, textModes[modeIdx].cols, textModes[modeIdx].rows);
                    }
                    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                break;
                default:
                    if(key.UnicodeChar == u'\r' && textModes[modeIdx].cols != 0) { // QEMU can have invalid text modes
                        cout->SetMode(cout, textModes[modeIdx].mode);
                        cout->QueryMode(cout, textModes[modeIdx].mode, &textModes[modeIdx].cols, &textModes[modeIdx].rows);

                        cout->ClearScreen(cout);
                        gettingInput = false;
                        modeIdx = 0;
                    }
                break;
            }
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
    UINTN modeIdx = 0; // Current mode within entire menu of GOP mode choices
    

    // store found GOP mode info
    typedef struct {
        UINT32 width;
        UINT32 height;
    } GopModeInfo;
    GopModeInfo gopModes[50];

    status = bs->LocateProtocol(&gopGuid, NULL, (VOID **)&gop);

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
            u"Framebuffer size: %x\r\n"
            u"PixelFormat: %d\r\n"
            u"PixelsPerScanLine: %u\r\n",
            gop->Mode->MaxMode,
            gop->Mode->Mode,
            modeInfo->HorizontalResolution, modeInfo->VerticalResolution,
            gop->Mode->FrameBufferBase,
            gop->Mode->FrameBufferSize,
            modeInfo->PixelFormat,
            modeInfo->PixelsPerScanLine
        );
        
        cout->OutputString(cout, u"\r\nAvailable GOP modes:\r\n");

        UINTN menuTop = cout->Mode->CursorRow, menuBottom = 0, maxCols;
        cout->QueryMode(cout, cout->Mode->Mode, &maxCols, &menuBottom);

        // Print keybinds at the bottom of the screen
        cout->SetCursorPosition(cout, 0, menuBottom-3);
        printf(u"Up/Down Arrow = Move\r\n"
               u"Enter = Select\r\n"
               u"Escape = Back");

        cout->SetCursorPosition(cout, 0, menuTop);
        menuBottom -= 5; // Bottom of menu will be 2 rows above keybinds
        UINTN menuLength = menuBottom - menuTop;

        // get all available GOP modes' info
        const UINT32 max = gop->Mode->MaxMode;
        for (UINT32 i = 0; i < ARR_SIZE(gopModes) && i < max; i++) {
            gop->QueryMode(gop, i, &modeInfoSize, &modeInfo);

            gopModes[i].width = modeInfo->HorizontalResolution;
            gopModes[i].height = modeInfo->VerticalResolution;
        }

        // Highlight top menu row to start off
        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
        printf(u"Mode %d: %dx%d", 0, gopModes[0].width, gopModes[0].height);

        // Print other GOP mode infos
        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
        for(UINTN i = 1; i < menuLength + 1; i++) {
            printf(u"\r\nMode %d: %dx%d", i, gopModes[i].width, gopModes[i].height);
        }

        // Get input from user
        cout->SetCursorPosition(cout, 0, menuTop);
        bool gettingInput = true;
        while(gettingInput) {
            UINTN currentRow = cout->Mode->CursorRow;
            EFI_INPUT_KEY key = getKey();

            switch(key.ScanCode) { // Scrolling logic
                case SCANCODE_ESC: return EFI_SUCCESS;
                case SCANCODE_UP_ARROW:
                    if(currentRow == menuTop && modeIdx > 0) {
                        // scroll menu up by decrementing all modes by 1
                        printf(u"                    \r");// blank out mode text first
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        modeIdx--;
                        printf(u"Mode %d: %dx%d", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height);

                        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                        UINTN tempMode = modeIdx + 1;
                        for(UINTN i = 0; i < menuLength; i++, tempMode++) {
                            printf(u"\r\n                    \r"
                                   u"Mode %d: %dx%d\r", tempMode, gopModes[tempMode].width, gopModes[tempMode].height);
                        }

                        cout->SetCursorPosition(cout, 0, menuTop);
                    } else if(currentRow-1 >= menuTop) {
                        // de-highlight current row,
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height);

                        modeIdx--;
                        currentRow--;

                        cout->SetCursorPosition(cout, 0, currentRow);
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height);
                    }
                    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                break;
                case SCANCODE_DOWN_ARROW: 
                    if(currentRow == menuBottom && modeIdx < max-1) {
                        // scroll menu up by incrementing all modes by 1
                        modeIdx -= menuLength - 1;
                        // reset cursor to top
                        cout->SetCursorPosition(cout, 0, menuTop);
                        // print modes up until last menu row
                        for(UINTN i = 0; i < menuLength; i++, modeIdx++) {
                            printf(u"                    \r"
                                   u"Mode %d: %dx%d\r\n", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height);
                        }

                        // highlight last row
                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height);

                    } else if (currentRow+1 <= menuBottom) {
                        // de-highlight current row,
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r\n", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height);

                        modeIdx++;
                        currentRow++;

                        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
                        printf(u"                    \r"
                               u"Mode %d: %dx%d\r", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height);
                    }
                    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
                break;
                default:
                    if(key.UnicodeChar == u'\r') {
                        // Enter key, set GOP mode.
                        gop->SetMode(gop, modeIdx);
                        gop->QueryMode(gop, modeIdx, &modeInfoSize, &modeInfo);

                        // Clear GOP screen
                        EFI_GRAPHICS_OUTPUT_BLT_PIXEL px = {0x00, 0x00, 0x00, 0x00};
                        gop->Blt(gop, &px, EfiBltVideoFill, 
                                 0, 0, // Source BLT BUFFER X,Y
                                 0, 0,  // Destination screen X,Y
                                 modeInfo->HorizontalResolution, modeInfo->VerticalResolution,
                                 0
                                );
                        gettingInput = false; // will leave input loop and redraw
                        modeIdx = 0; // Reset last selected mode in menu
                    }
                break;
            }
        }
    }
    return EFI_SUCCESS;
}
