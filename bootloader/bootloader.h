#pragma once
#include "efi.h"
#include "efilib.h"

// -----------------
//  Global macros
// -----------------
#define ARR_SIZE(x) (sizeof (x) / sizeof (x)[0])
#define BOOL_TO_YN(b) ((b) ? u"Yes" : u"No")
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
#define PAGE_SIZE 4096 
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B // PE32+ magic number

// -----------------
// Mouse drawing stuff
// -----------------
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
// memset for compling with clang/gcc (set len bytes of dst memory with int c) UNCOMMENT FOR CLANG
// =================
VOID *memset(VOID *dst, UINT8 c, UINTN len) {
  UINT8 *p = dst;
  for(UINTN i = 0; i < len; i++) {
    p[i] = c;
  }
  return dst;
}

// =================
// memcpy for compiling with clang/gcc (set len bytes of dst memory from src) UNCOMMENT FOR CLANG
// =================
VOID *memcpy(VOID *dst, VOID *src, UINTN len) {
  UINT8 *p = dst;
  UINT8 *q = src;
  for(UINTN i = 0; i < len; i++) {
    p[i] = q[i];
  }
  return dst;
}

// ================
// memcmp Compare up to len bytes of m1 and m2, stop at first point that they dont equal.
// ================
INTN memcmp(VOID *m1, VOID *m2, UINTN len) {
  UINT8 *p = m1;
  UINT8 *q = m2;
  for(UINTN i = 0; i < len; i++) {
    if(p[i] != q[i]) return (INTN)p[i] - (INTN)q[i];
  }
  return 0;
}

// =================
// Substring function
// =================
char *substr(char *haystack, char *needle) {
  if(!needle) return haystack;

  char *p = haystack;
  while(*p) {
    if(*p == *needle) {
      if(!memcmp(p, needle, strLen(needle))) return p;
    }
    p++;
  }
  return NULL;
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
// Read a fully qualified file path in the ESP into an output buffer.
// File path must start with root '\', escaped as needed by the caller with '\\'.
// Returns: non-null pointer to allocated buffer with file data, allocated with Boot Services AllocatePool(),
// or NULL if not found or error.
// NOTE: Caller will have to use FreePool() on returned buffer to free allocated memory.
// =================
VOID *readEspFileToBuffer(CHAR16 *path, UINTN *fileSize) {
  VOID *fileBuffer = NULL;
  EFI_STATUS status;

  // Get loaded image protocol first to grab device handle to use simple file system protocol on
  EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_LOADED_IMAGE_PROTOCOL *lip = NULL;
  status = bs->OpenProtocol(image, &lipGuid, (VOID **)&lip, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not open Loaded Image Protocol!\r\n", status);
    goto cleanup;
  }

  // Get Simple File System Protocol for device handle for this loaded image, to open the root directory for the ESP.
  EFI_GUID sfspGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp = NULL;
  status = bs->OpenProtocol(lip->DeviceHandle, &sfspGuid, (VOID **)&sfsp, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if(EFI_ERROR(status)) {
    error(u"Error %x: Could not open Simple FileSystem Protocol\r\n", status);
    goto cleanup;
  }

  // Open the root directory via OpenVolume()
  EFI_FILE_PROTOCOL *root = NULL;
  status = sfsp->OpenVolume(sfsp, &root);
  if(EFI_ERROR(status)) {
    error(u"Error %x: Could not open volume for root directory in ESP!\r\n", status);
    goto cleanup;
  }

  // Open fle in input path
  EFI_FILE_PROTOCOL *file = NULL;
  status = root->Open(root, &file, path, EFI_FILE_MODE_READ, 0);
  if(EFI_ERROR(status)) {
    error(u"Error %x: Could not open file '%s' for reading!\r\n", status, path);
    goto cleanup;
  }
  // Something is broken here
  // Get info for file, to grab size
  EFI_FILE_INFO fileInfo;
  EFI_GUID fileInfoGuid = EFI_FILE_INFO_ID;
  UINTN bufferSize = sizeof(EFI_FILE_INFO);
  status = file->GetInfo(file, &fileInfoGuid, &bufferSize, &fileInfo);
  if(EFI_ERROR(status)) {
    error(u"Error %x: Could not get file info for '%s'!\r\n", status, path);
    goto fileCleanup;
  }

  // Allocate buffer for file
  bufferSize = fileInfo.FileSize;
  status = bs->AllocatePool(EfiLoaderData, bufferSize, &fileBuffer);
  if(EFI_ERROR(status) || bufferSize != fileInfo.FileSize) {
    error(u"Error %x: Could not allocate memory for '%s'!\r\n", status, path);
    goto fileCleanup;
  }

  if(EFI_ERROR(status)) {
    error(u"Error %x: Could not get file info for file %s!\r\n", status, path);
    goto fileCleanup;
  } 

  // Read file into buffer
  status = file->Read(file, &bufferSize, fileBuffer);
  if(EFI_ERROR(status) || bufferSize != fileInfo.FileSize) {
    error(u"Error %x: Could not read file '%s' into buffer!\r\n", status, path);
    goto fileCleanup;
  }
  
  // Set output file size in buffer
  *fileSize = bufferSize;
  
  fileCleanup:
  // Close open file/dir pointers
  root->Close(root);
  file->Close(file);

  cleanup:
  // Close open protocols
  bs->CloseProtocol(lip->DeviceHandle, &sfspGuid, image, NULL);
  bs->CloseProtocol(image, &lipGuid, image, NULL);
  return fileBuffer; // will return buffer with file data, or NULL if error
}

// =================
// Read disk LBA to buffer
// Reads a single LBA (Logical Block Address) from the disk into a buffer.
// =================
VOID *readDiskLbasToBuffer(EFI_LBA diskLba, UINTN dataSize, UINT32 diskMediaID) {
  VOID *buffer = NULL;
  EFI_STATUS status = EFI_SUCCESS;

  // Loop through and get Block IO Protocol for input media ID, for entire disk. 
  // NOTE: This assumnes that the first block io found with logical partition false is the disk entirely itself.
  EFI_GUID bipGuid = EFI_BLOCK_IO_PROTOCOL_GUID;
  EFI_BLOCK_IO_PROTOCOL *bip = NULL;
  UINTN numHandles = 0;
  EFI_HANDLE *handleBuffer = NULL;
  
  status = bs->LocateHandleBuffer(ByProtocol, &bipGuid, NULL, &numHandles, &handleBuffer);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x Could not locate Block IO Protocol handles!\r\n", status);
    return buffer;
  }

  BOOLEAN found = FALSE;
  UINTN i = 0;
  for (i = 0; i < numHandles; i++) {
    // Open Block IO Protocol for each handle
    status = bs->OpenProtocol(handleBuffer[i], &bipGuid, (VOID **)&bip, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if(EFI_ERROR(status)) {
      error(u"ERROR: %x Could not open Block IO Protocol for handle %u!\r\n", status, i);
      bs->CloseProtocol(handleBuffer[i], &bipGuid, image, NULL);
      continue; // Try next handle
    }

    if(bip->Media->MediaId == diskMediaID && !bip->Media->LogicalPartition){
      found = TRUE;
      break;
    }
    bs->CloseProtocol(handleBuffer[i], &bipGuid, image, NULL);
  }

  if(!found) {
    error(u"ERROR: %x; Could not find Block IO Protocol for disk with ID: %u!\r\n", status, diskMediaID);
    return buffer;
  } 

  // Get Disk IO Protocol on the same device as block IO Protocol
  EFI_GUID dipGuid = EFI_DISK_IO_PROTOCOL_GUID;
  EFI_DISK_IO_PROTOCOL *dip = NULL;

  status = bs->OpenProtocol(handleBuffer[i], &dipGuid, (VOID **)&dip, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  if(EFI_ERROR(status)) {
    error(u"ERROR: %x Could not open Disk IO Protocol on handle: %u!\r\n", status, i);
    goto cleanup;
  }

  // Allocate buffer for data
  status = bs->AllocatePool(EfiLoaderData, dataSize, &buffer);
  if(EFI_ERROR(status) || buffer == NULL) {
    error(u"ERROR: %x Could not allocate buffer for disk data!\r\n", status);
    bs->CloseProtocol(handleBuffer[i], &dipGuid, image, NULL);
    goto cleanup;
  }

  // Use Disk IO Read to read into allocated buffer
  status = dip->ReadDisk(dip, diskMediaID, diskLba * bip->Media->BlockSize, dataSize, buffer);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x Could not read disk LBAs into buffer!\r\n", status);
  }

  bs->CloseProtocol(handleBuffer[i], &dipGuid, image, NULL); // Close Block IO Protocol for this handle

  cleanup: 
  bs->CloseProtocol(handleBuffer[i], &bipGuid, image, NULL);
  return buffer; // will return buffer with LBA data, or NULL if error
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
    error(u"ERROR: %x Could not locate GOP! :(\r\n", status);
    return status;
  }

  // Overall Screen Loop
  while(true) {
    cout->ClearScreen(cout);

    printf(u"Graphics mode information:\r\n");

    //Get current GOP mode information
    status = gop->QueryMode(gop, gop->Mode->Mode, &modeInfoSize, &modeInfo);

    if(EFI_ERROR(status)) {
      error(u"ERROR: %x Could not Query GOP Mode %u\r\n", status, gop->Mode->Mode);
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
    error(u"ERROR: %x Could not locate GOP! :(\r\n", status);
    return status;
  }

  gop->QueryMode(gop, gop->Mode->Mode, &modeInfoSize, &modeInfo);

  // Use LocateHandleBuffer() to find all SPP and get a valid one.
  status = bs->LocateHandleBuffer(ByProtocol, &sppGuid, NULL, &numHandles, &handleBuffer);
  if (EFI_ERROR(status)) {
    error(u"\r\nERROR: %x; Could not locate SPP handle buffer!\r\n", status);
  }

  cout->ClearScreen(cout);

  BOOLEAN foundMode = FALSE;
  // Open all SPP for each handle.
  for (UINTN i=0; i < numHandles; i++) {
    status = bs->OpenProtocol(handleBuffer[i], &sppGuid, (VOID **)&spp[i], image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if(EFI_ERROR(status)) {
      error(u"\r\nERROR: %x; Could not Open SPP on handle!\r\n", status);
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

  if(!foundMode) error(u"\r\nERROR: Could not find any valid SPP mode!\r\n");  

  // Free memory pool allocated by LocateHandleBuffer()
  bs->FreePool(handleBuffer);

  // Use LocateHandleBuffer() to find all APP and get a valid one.
  numHandles = 0;
  handleBuffer = NULL;
  foundMode = FALSE;

  status = bs->LocateHandleBuffer(ByProtocol, &appGuid, NULL, &numHandles, &handleBuffer);
  if (EFI_ERROR(status)) {
    error(u"\r\nERROR: %x; Could not locate APP handle buffer!\r\n", status);
  }

  // Open all APP for each handle.
  for (UINTN i=0; i < numHandles; i++) {
    status = bs->OpenProtocol(handleBuffer[i], &appGuid, (VOID **)&app[i], image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if(EFI_ERROR(status)) {
      error(u"\r\nERROR: %x; Could not Open APP on handle!\r\n", status);
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

  if(!foundMode) error(u"\r\nERROR: Could not find any valid APP mode!\r\n");

  if(numProtocols == 0 ) {
    error(u"\r\nERROR: Could not find any S/APPs!\r\n");
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
    error(u"Error %x: Could not open Loaded Image Protocol\r\n", status);
    return status;
  }

  // Get Simple File System Protocol for device handle for this loaded image, to open the root directory for the ESP.
  EFI_GUID sfspGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp = NULL;

  status = bs->OpenProtocol(lip->DeviceHandle, &sfspGuid, (VOID **)&sfsp, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  
  if(EFI_ERROR(status)) {
    error(u"Error %x: Could not open Simple FileSystem Protocol\r\n", status);
    return status;
  }
  
  // Open root directory via OpenVolume()
  EFI_FILE_PROTOCOL *dirp = NULL;
  status = sfsp->OpenVolume(sfsp, &dirp);

  if(EFI_ERROR(status)) {
    error(u"Error %x: Could not open volume for root directory\r\n", status);
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
        dirp->Close(dirp);
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
        if (key.UnicodeChar == u'\r') {
          // Enter key:
          //  for a directory, enter that directory and iterate the loop.
          //  for a file, print file contents on screen.
          // Get directory entry under cursor row
          dirp->SetPosition(dirp, 0);
          INT32 i = 0;
          do {
            buffSize = sizeof fileInfo;
            dirp->Read(dirp, &buffSize, &fileInfo);
            i++;
          } while (i < csrRow);

          // open directory
          if(fileInfo.Attribute & EFI_FILE_DIRECTORY) {
            EFI_FILE_PROTOCOL *newDir;
            status = dirp->Open(dirp, &newDir, fileInfo.FileName, EFI_FILE_MODE_READ, 0);

            if(EFI_ERROR(status)) {
              error(u"Error %x: Could not open new directory %s\r\n", status, fileInfo.FileName);
              goto cleanup;
            }

            dirp->Close(dirp);
            dirp = newDir;

            csrRow = 1; // reeset user row to first entry in new directory

            // Set path for current dir
            if(!strcmp_u16(fileInfo.FileName, u".", 2)) {
              // Current directory, Do nothing
            } else if (!strcmp_u16(fileInfo.FileName, u"..", 3)) {
              // Parent directory, go back up.
              CHAR16 *pos = strrchr_u16(currentDirectory, u'/');
              if (pos == currentDirectory) pos++;
              *pos = u'\0';
            } else {
              // Go into nested directory, add onto current string
              if(currentDirectory[1] != u'\0') {
                strcat_u16(currentDirectory, u"/");
              }
              strcat_u16(currentDirectory, fileInfo.FileName);
            }
            continue; // Continue overall loop and print new dir entries
          } 

          // Else this is a file, print contents
          // Allocate buffer for file
          VOID *buffer = NULL;
          buffSize = fileInfo.FileSize;
          status = bs->AllocatePool(EfiLoaderData, buffSize, &buffer);

          if(EFI_ERROR(status)) {
              error(u"Error %x: Could not allocate memory for file %s\r\n", status, fileInfo.FileName);
              goto cleanup;
          }

          // Open File
          EFI_FILE_PROTOCOL *file;
          status = dirp->Open(dirp, &file, fileInfo.FileName, EFI_FILE_MODE_READ, 0);

          if(EFI_ERROR(status)) {
              error(u"Error %x: Could not open file %s\r\n", status, fileInfo.FileName);
              goto cleanup;
          }

          // Read file into buffer
          buffSize = fileInfo.FileSize;
          status = dirp->Read(file, &buffSize, buffer);
          if(EFI_ERROR(status)) {
              error(u"Error %x: Could not read file %s into buffer.\r\n", status, fileInfo.FileName);
              goto cleanup;
          }

          if(buffSize != fileInfo.FileSize) {
            error(
              u"Error: Could not read all file %s into buffer.\r\n"
              u"Bytes read: %u, Expected: %u\r\n",
              fileInfo.FileName,
              buffSize,
              fileInfo.FileSize
            );
            goto cleanup;
          }

          // Print buffer
          printf(u"\r\nFile Contents:\r\n");
          
          char *pos = (char *)buffer;
          for(UINTN bytes = buffSize; bytes > 0; bytes--) {
            CHAR16 str[2];
            str[0] = *pos;
            str[1] = u'\0';
            if(*pos == '\n') {
              // Convert LF newline to CRLF
              printf(u"\r\n", str);
            } else {
              printf(u"%s", str);
            }
            pos++;
          }
          printf(u"\r\n\r\nPress any key to continue...\r\n");
          getKey();
          

          // Free memory pool
          bs->FreePool(buffer);

          // Close file handle
          dirp->Close(file);
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

// =====================
// Get Media ID value for this running this image
// =====================
EFI_STATUS getDiskImageMediaID(UINT32 *mediaID) {
  EFI_STATUS status = EFI_SUCCESS;

  EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_LOADED_IMAGE_PROTOCOL *lip = NULL;
  status = bs->OpenProtocol(image, &lipGuid, (VOID **)&lip, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);

  if(EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not open Loaded Image protocol\r\n", status);
    goto done;
  }

  EFI_GUID bipGuid = EFI_BLOCK_IO_PROTOCOL_GUID;
  EFI_BLOCK_IO_PROTOCOL *bip = NULL;
  status = bs->OpenProtocol(lip->DeviceHandle, &bipGuid, (VOID **)&bip, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not open Block IO Protocol for this loaded image.\r\n", status);
    goto done;
  }

  *mediaID = bip->Media->MediaId;

  done:
  return status;
}

// =================
// Print Block IO Partitions using Block IO and Partition Info Protocols
// =================
EFI_STATUS printBlockIoPartitions(void) {
  EFI_STATUS status = EFI_SUCCESS;

  cout->ClearScreen(cout);

  EFI_BLOCK_IO_PROTOCOL *bip;
  EFI_GUID bipGuid = EFI_BLOCK_IO_PROTOCOL_GUID;
  UINTN numHandles = 0;
  EFI_HANDLE *handleBuffer = NULL;
  
  // Get media ID for this disk image first, to compare to others in output
  UINT32 thisImageMediaId = 0;
  status = getDiskImageMediaID(&thisImageMediaId);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not get disk image media ID.\r\n", status);
    return status;
  }

  // Loop through and print all partition info found
  status = bs->LocateHandleBuffer(ByProtocol, &bipGuid, NULL, &numHandles, &handleBuffer);
  if(EFI_ERROR(status)) {
    error(u"Error: %x; Could not locate Block IO Protocols.\r\n", status);
    return status;
  }
  
  UINT32 lastMediaId = -1; // Keep track of currently opened Media info
  for(UINTN i = 0; i < numHandles; i++){
    status = bs->OpenProtocol(handleBuffer[i], &bipGuid, (VOID **)&bip, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if(EFI_ERROR(status)) {
      error(u"Error: %x; Could not open any Block IO protocol on handle %u\r\n", status, i);
      continue;
    }

    // Print Block IO Media Info for this Disk/Partition
    if(lastMediaId != bip->Media->MediaId) {
      lastMediaId = bip->Media->MediaId;
      printf(
        u"Media ID %u %s", 
        lastMediaId, 
        (lastMediaId == thisImageMediaId ? u"(Disk Image)\r\n" : u"\r\n")
      );
    }
    
    if(bip->Media->LastBlock == 0) {
      bs->CloseProtocol(handleBuffer[i], &bipGuid, image, NULL);
      continue;
    }

    printf(
      u"Removable: %s, Present: %s, Log.Part.: %s, Read Only: %s, Write Caching: %s,\r\n"
      u"Block Size: %u, IO Alignment: %u, Last Block: %u, Lowest Aligned LBA: %u,\r\n"
      u"Log.Blocks/Phys.Block: %u, Optimal Transfer Length Granularity: %u\r\n",
      BOOL_TO_YN(bip->Media->RemovableMedia),
      BOOL_TO_YN(bip->Media->MediaPresent),
      BOOL_TO_YN(bip->Media->LogicalPartition),
      BOOL_TO_YN(bip->Media->ReadOnly),
      BOOL_TO_YN(bip->Media->WriteCaching),
      bip->Media->BlockSize,
      bip->Media->IoAlign,
      bip->Media->LastBlock,
      bip->Media->LowestAlignedLba,
      bip->Media->LogicalBlocksPerPhysicalBlock,
      bip->Media->OptimalTransferLengthGranularity
    );

    // Print type of partition e.g ESP or Data, or Other
    if (!bip->Media->LogicalPartition) printf(u"<Entire Disk>\r\n");
    else {
      // Get partition info protocol for this partition
      EFI_GUID pipGuid = EFI_PARTITION_INFO_PROTOCOL_GUID;
      EFI_PARTITION_INFO_PROTOCOL *pip = NULL;
      status = bs->OpenProtocol(handleBuffer[i], &pipGuid, (VOID **)&pip, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
      if(EFI_ERROR(status)) {
        error(u"\r\nError: %x; Could not open partition info protocol on handle %u.\r\n", status, i);
      } else {
        if      (pip->Type == PARTITION_TYPE_OTHER )  printf(u"<Other type>\r\n");
        else if (pip->Type == PARTITION_TYPE_MBR)     printf(u"<MBR>\r\n");
        else if (pip->Type == PARTITION_TYPE_GPT) {
          if (pip->System == 1) printf(u"<EFI System Partition>\r\n");
          else {
            // Compare Gpt.PartitionTypeGUID with known values
            EFI_GUID dataGuid = BASIC_DATA_GUID;
            if(!memcmp(&pip->Info.Gpt.PartitionTypeGUID, &dataGuid, sizeof(EFI_GUID))) printf(u"<Basic Data>\r\n");
            else printf(u"Other GPT Type>\r\n");
          }
        }
      }
    }
    printf(u"\r\n");
  }

  printf(u"Press any key to go back...");
  getKey();
  return status;
}

// =================
// Load ELF64 PIE File into a new buffer and return entry point for the loaded ELF Kernel
// =================
VOID *loadElf(VOID *elfBuffer) {
  ELF_Header_64 *ehdr = elfBuffer;

  // Print ELF Header info
  printf(
    u"Type: %u, Machine: %x, Entry: %x\r\n"
    u"PgmHdr Offset: %u, ELF Hdr Size: %u\r\n"
    u"Pgm Entry Size: %u, Num. PgmHdrs: %u\r\n",
    ehdr->e_type, ehdr->e_machine, ehdr->e_entry,
    ehdr->e_phoff, ehdr->e_ehsize, 
    ehdr->e_phentsize, ehdr->e_phnum
  );

  // Only allow PIE Files
  if(ehdr->e_type != ET_DYN) {
    error(u"ERROR: ELF file is not a PIE file, only ET_DYN/0x03 type is allowed.\r\n");
    return NULL;
  }

  // Print Loadable program header info for user, and get metrics for loading
  ELF_Program_Header_64 *phdr = (ELF_Program_Header_64 *)((UINT8 *)ehdr + ehdr->e_phoff);

  UINTN maxAlignment = PAGE_SIZE;
  UINTN memMin = UINT64_MAX, memMax = 0;

  printf(u"\r\nLoadable Program Headers:\r\n");
  for(UINT16 i = 0; i < ehdr->e_phnum; i++, phdr++) {
    if(phdr->p_type != PT_LOAD) continue;

    printf(
      u"%u: Offset: %x, Vaddr: %x, Paddr: %x\r\n"
      u"File Size: %x, Mem Size: %x, Align: %x\r\n",
      (UINTN)i,
      phdr->p_offset,
      phdr->p_vaddr,
      phdr->p_paddr,
      phdr->p_filesz,
      phdr->p_memsz,
      phdr->p_align
    );
   
    // Update max alignment as needed
    if(maxAlignment < phdr->p_align) maxAlignment = phdr->p_align;

    UINTN memBegin = phdr->p_vaddr;
    UINTN memEnd = phdr->p_vaddr + phdr->p_memsz + maxAlignment-1;

    memBegin &= ~(maxAlignment - 1); // Round down to nearest page
    memEnd &= ~(maxAlignment - 1); // Round up to nearest page

    if(memBegin < memMin) memMin = memBegin; // Round down to nearest page
    if(memEnd > memMax) memMax = memEnd; // Round up to nearest page
  }

  UINTN maxMemoryNeeded = memMax - memMin;
  printf(u"\r\nMemory needed for file: %x\r\n", maxMemoryNeeded);

  // Allocate buffer for program headers
  EFI_STATUS status = 0;
  VOID *programBuffer;
  status = bs->AllocatePool(EfiLoaderData, maxMemoryNeeded, &programBuffer);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not allocate memory for ELF program headers.\r\n", status);
    return NULL;
  }

  // Zero init buffer, to ensure 0 padding for all program sections
  memset(programBuffer, 0, maxMemoryNeeded);

  // Load program headers into buffer
  phdr = (ELF_Program_Header_64 *)((UINT8 *)ehdr + ehdr->e_phoff);
  for(UINT16 i = 0; i < ehdr->e_phnum; i++, phdr++) {
    if(phdr->p_type != PT_LOAD) continue;

    UINTN relativeOffset = phdr->p_vaddr - memMin;

    UINT8 *dst = (UINT8 *)programBuffer + relativeOffset;
    UINT8 *src = (UINT8 *)elfBuffer + phdr->p_offset;
    UINT32 len = phdr->p_memsz;
    memcpy(dst, src, len);
  }

  VOID *entryPoint = (VOID *)((UINT8 *)programBuffer + (ehdr->e_entry - memMin)); // Calculate entry point address in new buffer
  // Return entry point
  return entryPoint;
}

// =================
// Load PE32+ PIE File into a new buffer and return entry point for the loaded PE32+ Kernel
// =================
VOID *loadPe(VOID *peBuffer) {
  // Print PE Signature
  UINT8 peSigOffset = 0x3C; // From PE File Format
  UINT32 peSigPos = *(UINT32 *)((UINT8 *)peBuffer + peSigOffset); // Get PE signature offset
  UINT8 *peSig = (UINT8 *)peBuffer + peSigPos; // Get PE signature bytes

  printf(u"\r\nPE Signature: [%x][%x][%x][%x]\r\n", 
    (UINTN)peSig[0], 
    (UINTN)peSig[1], 
    (UINTN)peSig[2], 
    (UINTN)peSig[3]
  );
  // Print Coff File Header Info
  PE_Coff_File_Header_64 *coffHeader = (PE_Coff_File_Header_64 *)(peSig + 4);
  printf(
    u"Machine: %x, NumberOfSections: %u\r\n"
    u"SizeOfOptHdr: %u, Characteristics: %x\r\n",
    coffHeader->Machine,
    coffHeader->NumberOfSections,
    coffHeader->SizeOfOptionalHeader,
    coffHeader->Characteristics
  );

  // Validate some data
  if(coffHeader->Machine != IMAGE_FILE_MACHINE_AMD64) {
    error(u"ERROR: PE file is not a 64-bit file, only AMD64/0x8664 type is allowed.\r\n");
    return NULL;
  }

  if(!(coffHeader->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
    error(u"ERROR: File is not an executable.\r\n");
    return NULL;
  }

  // Print Optional Header Info
  PE_Optional_Header_64 *optHeader = (PE_Optional_Header_64 *)((UINT8 *)coffHeader + sizeof(PE_Coff_File_Header_64));
  printf(
    u"\r\nOptional Header:\r\n"
    u"Magic: %x, Entry Point: %x\r\n" 
    u"Sect Align: %x, File Align: %x, Size of Image: %x\r\n"
    u"Subsystem: %x, DllCharacteristics: %x\r\n",
    optHeader->Magic,
    optHeader->AddressOfEntryPoint,
    optHeader->SectionAlignment,
    optHeader->FileAlignment,
    optHeader->SizeOfImage,
    (UINTN)optHeader->Subsystem,
    (UINTN)optHeader->DllCharacteristics
  );

  // Validate info
  if(optHeader->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    error(u"ERROR: File not a PE32+ file.\r\n");
    return NULL;
  }

  if(!(optHeader->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE)) {
    error(u"ERROR: File not a PIE.\r\n");
    return NULL;
  }

  // Allocate buffer to load sections into
  EFI_STATUS status = 0;
  VOID *progBuffer = NULL;
  status = bs->AllocatePool(EfiLoaderData, optHeader->SizeOfImage, &progBuffer);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not allocate memory for PE file.\r\n", status);
    return NULL;
  }

  // Initialize buffer to 0, which should also take care of needing to 0-pad sections between raw data and virtual size
  memset(progBuffer, 0, optHeader->SizeOfImage);

  // Print section headers
  PE_Section_Header_64 *shdr = (PE_Section_Header_64 *)((UINT8 *)optHeader + coffHeader->SizeOfOptionalHeader);

  printf(u"\r\nSection Headers:\r\n");
  for(UINT16 i = 0; i < coffHeader->NumberOfSections; i++, shdr++) {
    printf(u"Name: ");
    char *pos = (char *)&shdr->Name;
    for (UINT8 j = 0; j < 8; j++) {
      CHAR16 str[2];
      str[0] = *pos;
      str[1] = u'\0';
      if(*pos == '\0') break; // 1488th line of code?? hitler reference? :D
      printf(u"%s", str);
      pos++;
    }

    printf(
      u" VSize: %x, VAddr: %x, DataSize: %x, DataPtr: %x\r\n",
      shdr->VirtualSize, 
      shdr->VirtualAddress, 
      shdr->SizeOfRawData, 
      shdr->PointerToRawData
    );
  }

  // Load sections into new buffer
  shdr = (PE_Section_Header_64 *)((UINT8 *)optHeader + coffHeader->SizeOfOptionalHeader);
  for(UINT16 i = 0; i < coffHeader->NumberOfSections; i++, shdr++) {
    if(shdr->SizeOfRawData == 0) continue;

    VOID *dst = (UINT8 *)progBuffer + shdr->VirtualAddress;
    VOID *src = (UINT8 *)peBuffer + shdr->PointerToRawData;
    UINTN len = shdr->SizeOfRawData;

    memcpy(dst, src, len);
  }
  
  // Return entry point
  VOID *entryPoint = (UINT8 *)progBuffer + optHeader->AddressOfEntryPoint; 
  return entryPoint;
}

// =================
// Get Memory Map from UEFI
// =================
EFI_STATUS getMemoryMap(MemoryMapInfo *mMap) {
  memset(mMap, 0, sizeof *mMap);
  // Get initial memory map size (send 0 for map size)
  EFI_STATUS status = EFI_SUCCESS;
  status = bs->GetMemoryMap(
    &mMap->size,
    mMap->map,
    &mMap->key,
    &mMap->descriptorSize,
    &mMap->descriptorVersion
  );

  if(EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL) {
    error(u"ERROR: %x; Could not get initial memory map size\r\n", status);
    return status;
  }

  // Allocate buffer for actual memory map size
  mMap->size += mMap->descriptorSize * 2; // Allocate enough space for an additional memory descriptor or 2 in the map due to this allocation itself.
  status = bs->AllocatePool(EfiLoaderData, mMap->size ,(VOID **)&mMap->map);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not allocate buffer for memory map size\r\n", status);
    return status;
  }

  // Call it again to get the actual memory map now that the buffer is the correct size
  status = bs->GetMemoryMap(
    &mMap->size,
    mMap->map,
    &mMap->key,
    &mMap->descriptorSize,
    &mMap->descriptorVersion
  );

  if(EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL) {
    error(u"ERROR: %x; Could not get UEFI memory map!\r\n", status);
    return status;
  }

  
  return EFI_SUCCESS;
}

// =================
// Print Memory Map
// =================
EFI_STATUS printMemoryMap(void) {
  cout->ClearScreen(cout);

  bs->CloseEvent(timerEvent);

  MemoryMapInfo mMap = {0};
  getMemoryMap(&mMap);
  
  // Print memory map descriptor values
  printf(
    u"Memory Map Size: %u, # Descriptor Size: %u\r\n" 
    u"Number of Descriptors: %u, Key: %x\r\n",
    mMap.size, mMap.descriptorSize,
    mMap.size / mMap.descriptorSize, mMap.key
  );

  UINTN usableBytes = 0; // "Usable" memory for an OS or similar, not firmware/device reserved
  for (UINTN i = 0; i < mMap.size / mMap.descriptorSize; i++) {
    EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)mMap.map + (i * mMap.descriptorSize));
    printf(
      u"%u: Typ: %u, Phy: %x, Vrt: %x, Pgs: %u, Att: %x\r\n",
      i,
      desc->Type,
      desc->PhysicalStart,
      desc->VirtualStart,
      desc->NumberOfPages,
      desc->Attribute
    );

    if(
      desc->Type == EfiResetShutdown ||
      desc->Type == EfiLoaderCode ||
      desc->Type == EfiLoaderData ||
      desc->Type == EfiBootServicesCode ||
      desc->Type == EfiBootServicesData ||
      desc->Type == EfiConventionalMemory ||
      desc->Type == EfiPersistentMemory
    ) {
      usableBytes += desc->NumberOfPages * 4096;
    }

    // Pause every 20 lines
    if (i > 0 && i % 20 == 0) getKey();
    
  }
  printf(
    u"\r\nUsable memory: %u / %u MiB / %u GiB\r\n",
    usableBytes, usableBytes / (1024 * 1024), usableBytes / (1024 * 1024 * 1024)
  );

  bs->FreePool(mMap.map);

  printf(u"Press any key to go back...");
  getKey();
  return EFI_SUCCESS;
}

// =================
// Read Files from Data Partition
// =================
EFI_STATUS loadKernel(void) {

  VOID *diskBuffer = NULL;
  VOID *fileBuffer = NULL;
  EFI_STATUS status = EFI_SUCCESS;
  UINTN fileSize = 0;
  UINTN diskLba = 0;
  char *strPos = NULL;
  // Print file info for TEST.TXT from path /EFI/BOOT/TEST.TXT
  CHAR16 *fileName = u"\\EFI\\BOOT\\FILE.TXT";

  cout->ClearScreen(cout);

  UINTN bufferSize = 0;
  fileBuffer = readEspFileToBuffer(fileName, &bufferSize);
  if(fileBuffer == NULL) {
    error(u"ERROR: Could not find or read %s from data partition to buffer.\r\n", fileName);
    goto exit;
  }

  // Search for file based on file name
  strPos = substr(fileBuffer, "kernel");
  if(!strPos) {
    error(u"ERROR: Could not find kernel file in data partition.\r\n");
    goto cleanup;
  }

  printf(u"KERNEL INFORMATION:\r\n");

  // Parse data from TEST.TXT file to get disk LBA and file size
  strPos = substr(fileBuffer, "FILE_SIZE=");
  if(!strPos) {
    error(u"ERROR: Could not find file size for file %s.\r\n", fileName);
    goto cleanup;
  }

  // Use an atoi function instead?
  strPos += strLen("FILE_SIZE=");
  while (isDigit(*strPos)) {
    fileSize = fileSize * 10 + *strPos - '0'; // Convert char -> int, add next decimal digit to number
    strPos++;
  }

  strPos = substr(fileBuffer, "DISK_LBA=");
  if(!strPos) {
    error(u"ERROR: Could not find disk lba value from buffer %s.\r\n", fileName);
    goto cleanup;
  }

  // Use an atoi function instead?
  strPos += strLen("DISK_LBA=");
  while (isDigit(*strPos)) {
    diskLba = diskLba * 10 + *strPos - '0'; // Convert char -> int, add next decimal digit to number
    strPos++;
  }

  printf(u"Found file size: %u bytes. Disk LBA: %u\r\n", fileSize, diskLba);

  // Get media ID(disk num for Block IO Protocol Media) for this running disk image
  UINT32 imageMediaID = 0;
  status = getDiskImageMediaID(&imageMediaID);

  if (EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not find or get MediaID value for disk image", status);
    bs->FreePool(fileBuffer);
    goto exit;
  }

  // Read disk lbas for file into buffer
  diskBuffer = readDiskLbasToBuffer(diskLba, fileSize, imageMediaID);
  if(!diskBuffer) {
    error(u"ERROR: Could not find or read data partition file to buffer.\r\n");
    bs->FreePool(diskBuffer); // Free memory allocated for disk LBA buffer
    goto exit;
  }

  // Get GOP info for kernel params
  EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;

  status = bs->LocateProtocol(&gopGuid, NULL, (VOID **)&gop);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not locate GOP for kernel parameters.\r\n", status);
    return status;
  }
  
  // Initialize Kernel params
  KernelParameters kparams = {0}; // Defined in efilib.h
  
  kparams.gopMode = *gop->Mode; // Copy GOP mode info to kernel params;
  void EFIAPI (*entryPoint)(KernelParameters) = NULL;

  // Load Kernel File depending on format (initial header bytes)
  UINT8 *hdr = diskBuffer;
  printf(u"File Format: ");
  printf(u"Header bytes: [%x][%x][%x][%x]\r\n", 
    hdr[0], 
    hdr[1], 
    hdr[2], 
    hdr[3]
  );
  
  if(!memcmp(hdr, (UINT8[4]){0x7F, 'E', 'L', 'F'}, 4)) {
    *(void **)&entryPoint = loadElf(diskBuffer);
  } else if(!memcmp(hdr, (UINT8[2]){'M', 'Z'}, 2)) {
    *(void **)&entryPoint = loadPe(diskBuffer);
  } else {
    printf(u"No header bytes, assuming this is a flat binary file.\r\n");
    *(void **)&entryPoint = diskBuffer; // get around compiler warning about function vs void pointer
  }

  if(!entryPoint) goto cleanup;

  printf(u"Press any key to load kernel...\r\n");
  getKey();

  // Close timer event so that it does not continue to fire off
  bs->CloseEvent(timerEvent);

  cout->ClearScreen(cout);

  status = bs->AllocatePool(EfiLoaderData, sizeof(MemoryMapInfo), (VOID **)&kparams.mMap);
  if(EFI_ERROR(status)) {
    error(u"ERROR: %x; Could not allocate MemoryMapInfo struct\r\n", status);
    goto cleanup;
  }

  // Get Memory Map
  if(EFI_ERROR(getMemoryMap(kparams.mMap))) goto cleanup;

  // Exit boot services before calling kernel
  if(EFI_ERROR(bs->ExitBootServices(image, kparams.mMap->key))) {
    error(u"ERROR: %x; Could not exit boot services!\r\n", status);
    goto cleanup;
  }

  // Call kernel entry point with parameters
  entryPoint(kparams);

  // Should not return to this point!
  __builtin_unreachable();
  
  // Final cleanup
  cleanup:
  bs->FreePool(fileBuffer); // Free memory allocated for ESP file
  bs->FreePool(diskBuffer); // Free memory allocated for disk LBA buffer

  exit:
  printf(u"\r\nPress any key to continue...\r\n");
  getKey();

  // Read and print actial file info from data partition
  return EFI_SUCCESS;
}