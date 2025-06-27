#pragma once
#include "efi.h"
#include <stdbool.h>
#include <stdarg.h>

// -----------------
//  Global macros
// -----------------
#define ARR_SIZE(x) (sizeof (x) / sizeof (x)[0])
#define PAD0(x) ((x) < 10 ? u"0" : u"")

// -----------------
//  Global constants
// -----------------
#define DEFAULT_FG_COLOR EFI_RED
#define DEFAULT_BG_COLOR EFI_BLACK
#define HL_FG_COLOR EFI_BLACK
#define HL_BG_COLOR EFI_LIGHTGRAY
#define px_LGRAY { 0xEE, 0xEE, 0xEE, 0x00 }
#define px_BLACK { 0x00, 0x00, 0x00, 0x00 }

// -----------------
//  Global vars
// -----------------
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout = NULL; // Console out
EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL; // Console out
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cerr = NULL; // Console error
EFI_BOOT_SERVICES *bs; // Boot Services
EFI_RUNTIME_SERVICES *rs; // Runtime Services
EFI_HANDLE image = NULL; // Image handle

// Mouse cursor buffer 8x8
EFI_GRAPHICS_OUTPUT_BLT_PIXEL cursorBuffer[] = {
    px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, // Line 1
    px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, // Line 2
    px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_BLACK, px_BLACK, // Line 3
    px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_BLACK, // Line 4
    px_LGRAY, px_LGRAY, px_BLACK, px_LGRAY, px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, // Line 5
    px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_LGRAY, px_LGRAY, px_LGRAY, px_BLACK, // Line 6
    px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_BLACK, px_LGRAY, px_LGRAY, px_LGRAY, // Line 7
    px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_BLACK, px_BLACK, px_LGRAY, px_LGRAY, // Line 8
};
// Buffer to save FB data at cursor position
EFI_GRAPHICS_OUTPUT_BLT_PIXEL savedBuffer[8*8] = {0};

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
// Copy string UINT16 strcpy
// =================
CHAR16 *strcpy_u16(CHAR16 *dst, CHAR16 *src) {
  if(!dst) return NULL;
  if(!src) return dst;

  CHAR16 *result = dst;
  while(*src) {
    *dst++ = *src++;
  }
  *dst = u'\0'; // Null terminate

  return result;
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
        case u'c': {
          cstr[0] = va_arg(args, int);
          cerr->OutputString(cerr, cstr);
        }
        break;
        case u's': {
          //printf("%s", string) - Print CHAR16 string
          CHAR16 *string = va_arg(args, CHAR16 *);
          cerr->OutputString(cerr, string);
        }
        break;
        case u'b': {
          UINTN number = va_arg(args, UINTN);
          eprintNum(number, 2, FALSE);
        }
        break;
        case u'o': {
          UINTN number = va_arg(args, UINTN);
          eprintNum(number, 8, FALSE);
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
        break;
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
        case u'c': {
          cstr[0] = va_arg(args, int);
          cout->OutputString(cout, cstr);
        }
        break;
        case u's': {
          CHAR16 *string = va_arg(args, CHAR16*);
          cout->OutputString(cout, string);
        }
        break;
        case u'b': {
          UINTN number = va_arg(args, UINTN);
          printNum(number, 2, FALSE);
        }
        break;
        case u'o': {
          UINTN number = va_arg(args, UINTN);
          printNum(number, 8, FALSE);
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
    UINT32 rows;
    UINT32 cols;
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
    printf(
      u"Max Mode: %d\r\n"
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
    cout->QueryMode(cout, cout->Mode->Mode, &maxCols, &menuBottom);

    // Print keybinds at the bottom of the screen
    cout->SetCursorPosition(cout, 0, menuBottom-3);
    printf(
      u"Up/Down Arrow = Move\r\n"
      u"Enter = Select\r\n"
      u"Escape = Back"
    );

    cout->SetCursorPosition(cout, 0, menuTop);
    menuBottom -= 5;
    UINTN menuLength = menuBottom - menuTop;

    // Print other text infos
    const UINT32 max = cout->Mode->MaxMode;
    if(max < menuLength) menuBottom = menuTop + max-1;
    for (UINT32 i = 0; i < ARR_SIZE(textModes) && i < max; i++) {
      cout->QueryMode(cout, i, &maxCols, &maxRows);
      textModes[i].rows = maxRows;
      textModes[i].cols = maxCols;
    }
    menuLength = menuBottom - menuTop;

    cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
    printf(u"Mode %d: %dx%d", 0, textModes[0].rows, textModes[0].cols);

    cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
    for(UINTN i = 1; i < menuLength + 1; i++) {
      printf(u"\r\nMode %d: %dx%d", i, textModes[i].rows, textModes[i].cols);
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
            printf(u"Mode %d: %dx%d", modeIdx, textModes[modeIdx].rows, textModes[modeIdx].cols);

            cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
            UINTN tempMode = modeIdx + 1;
            for(UINTN i=0; i<menuLength; i++, tempMode++) {
              printf(
                u"\r\n                    \r"
                u"Mode %d: %dx%d\r", tempMode, textModes[tempMode].rows, textModes[tempMode].cols
              );
            }
            cout->SetCursorPosition(cout, 0, menuTop);
          } else if (currentRow-1 >= menuTop){
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r", modeIdx, textModes[modeIdx].rows, textModes[modeIdx].cols
            );
            modeIdx--;
            currentRow--;
            cout->SetCursorPosition(cout, 0, currentRow);
            cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r", modeIdx, textModes[modeIdx].rows, textModes[modeIdx].cols
            );
          }
          cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
        break;
        case SCANCODE_DOWN_ARROW:
          if(currentRow == menuBottom && modeIdx < max-1) {
            modeIdx -= menuLength -1;
            cout->SetCursorPosition(cout, 0, menuTop);
            for(UINTN i = 0; i < menuLength; i++, modeIdx++) {
              printf(
                u"                    \r"
                u"Mode %d: %dx%d\r\n", modeIdx, textModes[modeIdx].rows, textModes[modeIdx].cols
              );
            }
            cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r", modeIdx, textModes[modeIdx].rows, textModes[modeIdx].cols
            );
          } else if(currentRow+1 <= menuBottom) {
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r\n", modeIdx, textModes[modeIdx].rows, textModes[modeIdx].cols
            );
            modeIdx++;
            currentRow++;
            cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r\n", modeIdx, textModes[modeIdx].rows, textModes[modeIdx].cols
            );
          }
          cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
        break;
        default:
          if(key.UnicodeChar == u'\r' && textModes[modeIdx].cols != 0) {
            cout->SetMode(cout, modeIdx);
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
    printf(
      u"Max Mode: %d\r\n"
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
    printf(
      u"Up/Down Arrow = Move\r\n"
      u"Enter = Select\r\n"
      u"Escape = Back"
    );

    cout->SetCursorPosition(cout, 0, menuTop);
    menuBottom -= 5; // Bottom of menu will be 2 rows above keybinds
    UINTN menuLength = menuBottom - menuTop;

    // get all available GOP modes' info
    const UINT32 max = gop->Mode->MaxMode;
    if (max < menuLength) {
      // bound menu by actual # of entries
      menuBottom = menuTop + max;
      menuLength = menuBottom - menuTop - 1; // Limit # of modes in menu to max modes - 1
    }
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
              printf(
                u"\r\n                    \r"
                u"Mode %d: %dx%d\r", tempMode, gopModes[tempMode].width, gopModes[tempMode].height
              );
            }
            cout->SetCursorPosition(cout, 0, menuTop);
          } else if(currentRow-1 >= menuTop) {
            // de-highlight current row,
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height
            );
            modeIdx--;
            currentRow--;
            cout->SetCursorPosition(cout, 0, currentRow);
            cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height
            );
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
              printf(
                u"                    \r"
                u"Mode %d: %dx%d\r\n", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height
              );
            }
            // highlight last row
            cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height
            );
          } else if (currentRow+1 <= menuBottom) {
            // de-highlight current row,
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r\n", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height
            );
            modeIdx++;
            currentRow++;
            cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
            printf(
              u"                    \r"
              u"Mode %d: %dx%d\r", modeIdx, gopModes[modeIdx].width, gopModes[modeIdx].height
            );
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
            gop->Blt(
              gop, &px, EfiBltVideoFill, 
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

// =================
// Testing Mouse, Touchscreen & various cursor/pointer support (Simple Pointer Protocol/Absolute Pointer Protocol)
// =================
EFI_STATUS testMouse(void) {
  // Get SPP via LocateHandleBuffer()
  EFI_GUID sppGuid = EFI_SIMPLE_POINTER_PROTOCOL_GUID;
  EFI_SIMPLE_POINTER_PROTOCOL *spp[5] = {0};
  UINTN numHandles = 0;
  EFI_HANDLE *handleBuffer = NULL;
  EFI_STATUS status = 0;
  INTN cursorSize = 8;           // Size in PX
  INTN cursorX = 0, cursorY = 0;  // Mouse cursor position

  // Get APP via LocateHandleBuffer()
  EFI_GUID appGuid = EFI_ABSOLUTE_POINTER_PROTOCOL_GUID;
  EFI_ABSOLUTE_POINTER_PROTOCOL *app[5] = {0};

  typedef enum {
    CIN = 0, // ConIn (kb)
    SPP = 1, // SimplePointerProtocol (mouse/touchpad)
    APP = 2, // AbsolutePointerProtocol (touchscreen/digitizer)
  } INPUT_TYPE;

  typedef struct {
    EFI_EVENT waitEvent; // This will be used in WaitForEvent()
    INPUT_TYPE type;
    union {
      EFI_SIMPLE_POINTER_PROTOCOL *spp;
      EFI_ABSOLUTE_POINTER_PROTOCOL *app;
    };
  } INPUT_PROTOCOL;

  INPUT_PROTOCOL inputProtocols[11] = {0}; // Max amount of inputProtocols 5spp 5app 1conin
  UINTN numProtocols = 0;

  // First input will be ConIn
  inputProtocols[numProtocols++] = (INPUT_PROTOCOL){
    .waitEvent = cin->WaitForKey,
    .type = CIN,
    .spp = NULL,
  };

  // Get GOP via LocateProtocol();
  EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *modeInfo = NULL;
  UINTN modeInfoSize = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  //UINTN modeIdx = 0; // Current mode within entire menu of GOP mode choices

  status = bs->LocateProtocol(&gopGuid, NULL, (VOID **)&gop);

  if(EFI_ERROR(status)) {
    eprintf(u"ERROR: %x Could not locate GOP! :(\r\n", status);
    return status;
  }

  gop->QueryMode(gop, gop->Mode->Mode, &modeInfoSize, &modeInfo);

  // Use LocateHandleBuffer() to find all SPP and get a valid one.
  status = bs->LocateHandleBuffer(ByProtocol, &sppGuid, NULL, &numHandles, &handleBuffer);
  if (EFI_ERROR(status)) {
    eprintf(u"\r\nERROR: %x; Could not locate SPP handle buffer!\r\n", status);
  }

  cout->ClearScreen(cout);

  BOOLEAN foundMode = FALSE;
  // Open all SPP for each handle.
  for (UINTN i=0; i < numHandles; i++) {
    status = bs->OpenProtocol(handleBuffer[i], &sppGuid, (VOID **)&spp[i], image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if(EFI_ERROR(status)) {
      eprintf(u"\r\nERROR: %x; Could not Open SPP on handle!\r\n", status);
      continue;
    }

    // Reset device
    spp[i]->Reset(spp[i], TRUE);

    // Print SPP info
    printf(
      u"SPP %u; Resolution X: %u, Y: %u, Z: %u, LeftButton: %u, RightButton: %u\r\n",
      i,
      spp[i]->Mode->ResolutionX,
      spp[i]->Mode->ResolutionY,
      spp[i]->Mode->ResolutionZ,
      spp[i]->Mode->LeftButton,
      spp[i]->Mode->RightButton
    );

    if(spp[i]->Mode->ResolutionX < 65535) {
      foundMode = TRUE;
      // Add valid protocol to array
      inputProtocols[numProtocols++] = (INPUT_PROTOCOL){
        .waitEvent = spp[i]->WaitForInput,
        .type = SPP,
        .spp = spp[i]
      };
    } // Found a valid mode
  }

  if(!foundMode) eprintf(u"\r\nERROR: Could not find any valid SPP mode!\r\n");  

  // Free memory pool allocated by LocateHandleBuffer()
  bs->FreePool(handleBuffer);

  // Use LocateHandleBuffer() to find all APP and get a valid one.
  numHandles = 0;
  handleBuffer = NULL;
  foundMode = FALSE;

  status = bs->LocateHandleBuffer(ByProtocol, &appGuid, NULL, &numHandles, &handleBuffer);
  if (EFI_ERROR(status)) {
    eprintf(u"\r\nERROR: %x; Could not locate APP handle buffer!\r\n", status);
  }

  // Open all APP for each handle.
  for (UINTN i=0; i < numHandles; i++) {
    status = bs->OpenProtocol(handleBuffer[i], &appGuid, (VOID **)&app[i], image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if(EFI_ERROR(status)) {
      eprintf(u"\r\nERROR: %x; Could not Open APP on handle!\r\n", status);
      continue;
    }

    // Reset device
    app[i]->Reset(app[i], TRUE);

    // Print APP info
    printf(
      u"\r\nAPP %u; Min X: %u, Y: %u, Z: %u, Max X: %u, Y: %u, Z: %u, Attributes: %b\r\n",
      i,
      app[i]->Mode->AbsoluteMinX,
      app[i]->Mode->AbsoluteMinY,
      app[i]->Mode->AbsoluteMinZ,
      app[i]->Mode->AbsoluteMaxX,
      app[i]->Mode->AbsoluteMaxY,
      app[i]->Mode->AbsoluteMaxZ,
      app[i]->Mode->Attributes
    );

    if(app[i]->Mode->AbsoluteMaxX < 65535) {
      foundMode = TRUE;
      // Add valid protocol to array
      inputProtocols[numProtocols++] = (INPUT_PROTOCOL){
        .waitEvent = app[i]->WaitForInput,
        .type = APP,
        .app = app[i]
      };
    } // Found a valid mode
  }

  if(!foundMode) eprintf(u"\r\nERROR: Could not find any valid APP mode!\r\n");

  if(numProtocols == 0 ) {
    eprintf(u"\r\nERROR: Could not find any S/APPs!\r\n");
    getKey();
    return 1;
  }

  // Found valid SPP mode, get mouse input

  // Start off in middle of screen
  INT32 xres = modeInfo->HorizontalResolution, yres = modeInfo->VerticalResolution;
  cursorX = (xres / 2) - (cursorSize / 2);
  cursorY = (yres / 2) - (cursorSize / 2);
  // Print initial mouse state & draw initial cursor
  printf(u"\r\nMouse Xpos: %d, Ypos: %dXmm: %d, Ymm: %d, LB: %u, RB: %u\r",
    cursorX,cursorY,0,0,0,0
  );

  // Draw cursor and save underlying FB data first
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *fb = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gop->Mode->FrameBufferBase;
  for ( INTN y = 0; y < cursorSize; y++) {
    for ( INTN x = 0; x < cursorSize; x++) {
      savedBuffer[(y * cursorSize) + x] = fb[(modeInfo->PixelsPerScanLine * (cursorY + y)) + (cursorX + x)];

      EFI_GRAPHICS_OUTPUT_BLT_PIXEL csrPx = cursorBuffer[(y * cursorSize) + x];
      fb[(modeInfo->PixelsPerScanLine * (cursorY + y)) + (cursorX + x)] = csrPx;
    }
  }

  // Input loop
  // Fill out event queue first
  EFI_EVENT events[11] = {0}; // Same # of elements as inputProtocols
  for (UINTN i = 0; i < numProtocols; i++) events[i] = inputProtocols[i].waitEvent;

  while(TRUE) {
    UINTN idx = 0;

    bs->WaitForEvent(numProtocols, events, &idx);
    if(inputProtocols[idx].type == CIN) {
      // Keypress
      EFI_INPUT_KEY key = { 0 };
      cin->ReadKeyStroke(cin, &key);

      if(key.ScanCode == SCANCODE_ESC) {
        // ESC, leave and go to main menu
        break;
      }
    } else if (inputProtocols[idx].type == SPP) {
      // Simple Pointer Protocol Mouse event
      // Get mouse state
      EFI_SIMPLE_POINTER_STATE state = { 0 };
      EFI_SIMPLE_POINTER_PROTOCOL *activeSpp = inputProtocols[idx].spp;
      activeSpp->GetState(activeSpp, &state);

      // Print current info
      // Movement is spp state's RelativeMovement / spp's Resolution
      // Movement amount is in mm; 1mm = 2% of X or Y Res

      float xmmFloat = (float)state.RelativeMovementX / (float)activeSpp->Mode->ResolutionX; 
      float ymmFloat = (float)state.RelativeMovementY / (float)activeSpp->Mode->ResolutionY;

      if(state.RelativeMovementX > 0 && xmmFloat == 0.0) xmmFloat = 1.0;
      if(state.RelativeMovementY > 0 && ymmFloat == 0.0) ymmFloat = 1.0;

      // Erase text first before reprinting
      printf(u"                                                                    \r"),
      printf(u"Mouse Xpos: %d, Ypos: %dXmm: %d, Ymm: %d, LB: %u, RB: %u\r",
        cursorX, cursorY, (INTN)xmmFloat, (INTN)ymmFloat, state.LeftButton, state.RightButton
      );

      // Draw cursor: get pixel amount to move per mm
      float xResMmPx = modeInfo->HorizontalResolution * 0.02;
      float yResMmPx = modeInfo->VerticalResolution * 0.02;

      // first overwrite current cursor position with screen bg color to "erase" cursor
      // save framebuffer data at mouse position first, then redraw that data instead of just overwriting with background color e.g. with Blt buffer and use EfiVideoToBltBuffer and EfiBltBufferToVideo
      fb = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gop->Mode->FrameBufferBase;
      for ( INTN y = 0; y < cursorSize; y++) {
        for ( INTN x = 0; x < cursorSize; x++) {
          fb[(modeInfo->PixelsPerScanLine * (cursorY + y)) + (cursorX + x)] = savedBuffer[(y * cursorSize) + x];
        }
      }

      cursorX += (INTN)(xResMmPx * xmmFloat);
      cursorY += (INTN)(yResMmPx * ymmFloat);

      // Keep cursor in screen bounds
      if (cursorX < 0) cursorX = 0;
      if (cursorX > xres - cursorSize) cursorX = xres - cursorSize;
      if (cursorY < 0) cursorY = 0;
      if (cursorY > yres - cursorSize) cursorY = yres - cursorSize;

      // save FB data at new cursor position
      for ( INTN y = 0; y < cursorSize; y++) {
        for ( INTN x = 0; x < cursorSize; x++) {
          savedBuffer[(y * cursorSize) + x] = fb[(modeInfo->PixelsPerScanLine * (cursorY + y)) + (cursorX + x)];

          EFI_GRAPHICS_OUTPUT_BLT_PIXEL csrPx = cursorBuffer[(y * cursorSize) + x];
          fb[(modeInfo->PixelsPerScanLine * (cursorY + y)) + (cursorX + x)] = csrPx;
        }
      }
    } else if (inputProtocols[idx].type == APP) {
      // Get state
      EFI_ABSOLUTE_POINTER_STATE state = { 0 };
      EFI_ABSOLUTE_POINTER_PROTOCOL *activeApp = inputProtocols[idx].app;
      activeApp->GetState(activeApp, &state);

      // Print state values:
      // Erase text first before reprinting
      printf(u"                                                                    \r"),
      printf(u"Pointer Xpos: %u, Ypos: %u Zpos: %u, Buttons: %b\r",
        state.CurrentX, state.CurrentY, state.CurrentZ, state.ActiveButtons
      );

      // save framebuffer data at mouse position first, then redraw that data instead of just overwriting with background color e.g. with Blt buffer and use EfiVideoToBltBuffer and EfiBltBufferToVideo
      fb = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gop->Mode->FrameBufferBase;
      for ( INTN y = 0; y < cursorSize; y++) {
        for ( INTN x = 0; x < cursorSize; x++) {
          fb[(modeInfo->PixelsPerScanLine * (cursorY + y)) + (cursorX + x)] = savedBuffer[(y * cursorSize) + x];
        }
      }

      // Get ratio of GOP res to APP max val to translate APP pos to correct onscreen GOP pos.

      float xAppRatio = (float)modeInfo->HorizontalResolution / (float)activeApp->Mode->AbsoluteMaxX;
      float yAppRatio = (float)modeInfo->VerticalResolution / (float)activeApp->Mode->AbsoluteMaxY;

      cursorX = (INTN)((float)state.CurrentX * xAppRatio);
      cursorY = (INTN)((float)state.CurrentY * yAppRatio);

      // Keep cursor in screen bounds
      if (cursorX < 0) cursorX = 0;
      if (cursorX > xres - cursorSize) cursorX = xres - cursorSize;
      if (cursorY < 0) cursorY = 0;
      if (cursorY > yres - cursorSize) cursorY = yres - cursorSize;

      // save FB data at new cursor position
      for ( INTN y = 0; y < cursorSize; y++) {
        for ( INTN x = 0; x < cursorSize; x++) {
          savedBuffer[(y * cursorSize) + x] = fb[(modeInfo->PixelsPerScanLine * (cursorY + y)) + (cursorX + x)];

          EFI_GRAPHICS_OUTPUT_BLT_PIXEL csrPx = cursorBuffer[(y * cursorSize) + x];
          fb[(modeInfo->PixelsPerScanLine * (cursorY + y)) + (cursorX + x)] = csrPx;
        }
      }
    }
  }

  return EFI_SUCCESS;
}

// =================
// Printing DateTime
// =================
VOID EFIAPI printDateTime(IN EFI_EVENT event, IN VOID *Context) {
  (VOID)event; // Suppress compiler warnings

  // Timer context will be the textmode screen bounds
  typedef struct {
    UINT32 rows, cols;
  } TimerContext;

  TimerContext context = *(TimerContext *)Context;

  // Save current cursor pos before printing
  UINT32 saveCol = cout->Mode->CursorColumn, saveRow = cout->Mode->CursorRow;

  // Get datetime
  EFI_TIME time = {0};
  EFI_TIME_CAPABILITIES capabilities = {0};

  // get current datetime
  rs->GetTime(&time, &capabilities);

  // Move cursor to print in lower right corner
  cout->SetCursorPosition(cout, context.cols - 20, context.rows -1);

  // Print current Datetime
  printf(u"%u-%s%u-%s%u %s%u:%s%u:%s%u",
    time.Year,
    PAD0(time.Month), time.Month,
    PAD0(time.Day),   time.Day,
    PAD0(time.Hour),  time.Hour,
    PAD0(time.Minute),time.Minute,
    PAD0(time.Second),time.Second
  );

  // Restore cursor pos
  cout->SetCursorPosition(cout, saveCol, saveRow);
}

// =================
// Reading ESP Files
// =================
EFI_STATUS readEspFiles(void) {
  // Get Loaded Image Protocol for this EFI image/app itself,
  // in order to get the device handle, to use for the Simple File System Protocol
  EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_LOADED_IMAGE_PROTOCOL *lip;
  EFI_STATUS status = EFI_SUCCESS;

  status = bs->OpenProtocol(image, &lipGuid, (VOID **)&lip, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  if(EFI_ERROR(status)) {
    eprintf(u"Error %x: Could not open Loaded Image Protocol\r\n", status);
    return status;
  }

  // Get Simple File System Protocol for device handle for this loaded image, to open the root directory for the ESP.
  EFI_GUID sfspGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp = NULL;

  status = bs->OpenProtocol(lip->DeviceHandle, &sfspGuid, (VOID **)&sfsp, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  
  if(EFI_ERROR(status)) {
    eprintf(u"Error %x: Could not open Simple FileSystem Protocol\r\n", status);
    return status;
  }
  
  // Open root directory via OpenVolume()
  EFI_FILE_PROTOCOL *dirp = NULL;
  status = sfsp->OpenVolume(sfsp, &dirp);

  if(EFI_ERROR(status)) {
    eprintf(u"Error %x: Could not open volume for root directory\r\n", status);
    goto cleanup;
  }

  // Start at root directory
  CHAR16 currentDirectory[256];
  strcpy_u16(currentDirectory, u"/");
  
  // Print dir entries for currently opened directory
  // Overall input loop
  INT32 csrRow = 1;
  while (true) {
    cout->ClearScreen(cout);
    printf(u"%s: \r\n", currentDirectory);

    INT32 numEntries = 0; 
    EFI_FILE_INFO fileInfo;

    dirp->SetPosition(dirp, 0); // Reset to start of directory entries
    UINTN buffSize = sizeof fileInfo;
    dirp->Read(dirp, &buffSize, &fileInfo);

    while(buffSize > 0) {
      // Got next dir entry, print info
      numEntries++;
      if(csrRow == cout->Mode->CursorRow){
        // Highlight row cursor/user is on
        cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
      }

      printf(u"%s %s\r\n", (fileInfo.Attribute & EFI_FILE_DIRECTORY) ? u"[DIR] " : u"[FILE]", fileInfo.FileName);

      if(csrRow+1 == cout->Mode->CursorRow){
        // Dehighlight row cursor/user is on
        cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
      }
      
      buffSize = sizeof fileInfo;
      dirp->Read(dirp, &buffSize, &fileInfo);
    }

    EFI_INPUT_KEY key = getKey();
    switch (key.ScanCode) {
      case SCANCODE_ESC: {
        // ESC key, go to main menu
        goto cleanup;
      }
      break;
      case SCANCODE_UP_ARROW: {
        if(csrRow > 1) csrRow--;
      }
      break;
      case SCANCODE_DOWN_ARROW: {
        if(csrRow < numEntries) csrRow++;
      }
      break;
      default: {
        if (key.UnicodeChar == u"\r") {
          // Enter key:
          //  for a directory, enter that directory and iterate the loop.
          //  for a file, print file contents on screen.
          // TODO:
          // Get directory entry under cursor row
          dirp->SetPosition(dirp, 0);

          do {
            buffSize = sizeof fileInfo;
            dirp->Read(dirp, &buffSize, &fileInfo);
          } while (cout->Mode->CursorRow < csrRow);
          
        }
      }
      break;
    }
  }

  getKey();

  cleanup:
  // Close open protocols
  bs->CloseProtocol(lip->DeviceHandle, &sfspGuid, image, NULL);
  bs->CloseProtocol(image, &lipGuid, image, NULL);
  
  return status;
} 