#include "efi.h"
#include <stdbool.h>
#include <stdarg.h>
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
// Print an integer (INT32 for now)
// =================
bool printInt(INT32 number) {
    const CHAR16 *digits = u"0123456789";
    CHAR16 buffer[11]; // enough for INT32_MAX + sign
    UINTN i = 0;
    const bool negative = (number < 0);

    if(negative) number = -number;

    do {
        buffer[i++] = digits[number % 10];
        number /= 10;
    } while(number > 0);

    // Prepend - when negatice
    if(negative) {
        buffer[i++] = u'-';
    }

    // null terminate string first
    buffer[i--] = u'\0';

    // reverse digits in buffer before printing
    for (UINTN j = 0; j < i; j++, i--) {
        // swap digits
        UINTN temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    // print number string
    cout->OutputString(cout, buffer);

    return true;
}

// =================
// Print a HEX Integer (UINTN)
// =================
bool printHex(UINTN number) {
    const CHAR16 *digits = u"0123456789ABCDEF";
    CHAR16 buffer[20]; // enough for UINTN_MAX, hopefully enough
    UINTN i = 0;

    do {
        buffer[i++] = digits[number % 16];
        number /= 16;
    } while(number > 0);

    // Prepend final string with 0x
    buffer[i++] = u'x';
    buffer[i++] = u'0';

    // null terminate string first
    buffer[i--] = u'\0';

    // reverse digits in buffer before printing
    for (UINTN j = 0; j < i; j++, i--) {
        // swap digits
        UINTN temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    // print number string
    cout->OutputString(cout, buffer);

    return true;
}




// =================
// Print formatted strings
// =================
bool printf(CHAR16 *fmt, ...) {
    bool result = false;
    CHAR16 cstr[2]; // TODO: place init this with memset and use = {} initializer
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
                    //printf("%d", number_int32) - Print INT32
                    INT32 number = va_arg(args, INT32);
                    printInt(number);
                }
                break;
                case u'x': {
                    //printf("%d", number_int32) - Print INT32
                    UINTN number = va_arg(args, UINTN);
                    printHex(number);
                }
                break;
                default:
                    cout->OutputString(cout, u"Invalid format specifier: %");
                    cstr[0] = fmt[i];
                    cout->OutputString(cout, cstr);
                    cout->OutputString(cout, u"\r\n");
                    result = false;
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

    result = true;
    return result;
} 

// =================
// EFI Image Entry Point
// =================
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle, (void)SystemTable; //Prevent compiler warning

    // Global vars init
    initGlbVars(SystemTable);
    
    // Reset console output
    cout->Reset(SystemTable->ConOut, false);

    cout->SetAttribute(SystemTable->ConOut, EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));

    cout->ClearScreen(SystemTable->ConOut);

    cout->OutputString(SystemTable->ConOut, u"Welcome to NervOS\r\n\r\n");

    
    printf(u"Testing hex: %x\r\n", (UINTN)0x11223344AABBCCDD);
    printf(u"Testing negative int: %d\r\n\r\n", (INT32)-54321);


    cout->OutputString(SystemTable->ConOut, u"Current text mode:\r\n");

    UINTN maxCols=0, maxRows=0;

    //Get current text mode's current cols and rows
    cout->QueryMode(cout, cout->Mode->Mode, &maxCols, &maxRows);

    printf(u"Max Mode: %d\r\n"
        u"Current Mode: %d\r\n"
        u"Attribute: %d\r\n" // TODO: change to %x and print 
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
    
    cout->OutputString(SystemTable->ConOut, u"Available text modes:\r\n\r\n");

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