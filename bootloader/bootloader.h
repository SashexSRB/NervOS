#pragma once
#include "efi.h"
#include "efilib.h"


// =================
// Set global vars
// =================
void initGlbVars(EFI_HANDLE handle, EFI_SYSTEM_TABLE *systable) {
  cout = systable->ConOut;
  cin = systable->ConIn;
  //cerr = systable->StdErr; // TODO: Research why it doesnt work in emulation
  cerr = cout; // Temporary
  st = systable;
  bs = st->BootServices;
  rs = st->RuntimeServices;
  image = handle;
}

pageTable *pml4 = NULL; // Top level 4 page level for x86-64 long mode paging

INT32 textRows = 0, textCols = 0;

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
            cout->QueryMode(cout, modeIdx, (UINTN *)&textModes[modeIdx].rows, (UINTN *)&textModes[modeIdx].rows);

            textRows = textModes[modeIdx].rows;
            textCols = textModes[modeIdx].rows;

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
    error(status, u"Could not open Loaded Image Protocol!\r\n");
    goto cleanup;
  }

  // Get Simple File System Protocol for device handle for this loaded image, to open the root directory for the ESP.
  EFI_GUID sfspGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp = NULL;
  status = bs->OpenProtocol(lip->DeviceHandle, &sfspGuid, (VOID **)&sfsp, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if(EFI_ERROR(status)) {
    error(status, u"Could not open Simple FileSystem Protocol\r\n");
    goto cleanup;
  }

  // Open the root directory via OpenVolume()
  EFI_FILE_PROTOCOL *root = NULL;
  status = sfsp->OpenVolume(sfsp, &root);
  if(EFI_ERROR(status)) {
    error(status, u"Could not open volume for root directory in ESP!\r\n");
    goto cleanup;
  }

  // Open fle in input path
  EFI_FILE_PROTOCOL *file = NULL;
  status = root->Open(root, &file, path, EFI_FILE_MODE_READ, 0);
  if(EFI_ERROR(status)) {
    error(status, u"Could not open file '%s' for reading!\r\n", path);
    goto cleanup;
  }
  // Something is broken here
  // Get info for file, to grab size
  EFI_FILE_INFO fileInfo;
  EFI_GUID fileInfoGuid = EFI_FILE_INFO_ID;
  UINTN bufferSize = sizeof(EFI_FILE_INFO);
  status = file->GetInfo(file, &fileInfoGuid, &bufferSize, &fileInfo);
  if(EFI_ERROR(status)) {
    error(status, u"Could not get file info for '%s'!\r\n", path);
    goto fileCleanup;
  }

  // Allocate buffer for file
  bufferSize = fileInfo.FileSize;
  status = bs->AllocatePool(EfiLoaderData, bufferSize, &fileBuffer);
  if(EFI_ERROR(status) || bufferSize != fileInfo.FileSize) {
    error(status, u"Could not allocate memory for '%s'!\r\n", path);
    goto fileCleanup;
  }

  if(EFI_ERROR(status)) {
    error(status, u"Could not get file info for file %s!\r\n", path);
    goto fileCleanup;
  } 

  // Read file into buffer
  status = file->Read(file, &bufferSize, fileBuffer);
  if(EFI_ERROR(status) || bufferSize != fileInfo.FileSize) {
    error(status, u"Could not read file '%s' into buffer!\r\n", path);
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
// Reads a single LBA (Logical Block Address) from the disk into a buffer. If executable input param is true, then allocate EfiLoaderCode, otherwise EfiLoaderData
// =================
EFI_PHYSICAL_ADDRESS readDiskLbasToBuffer(EFI_LBA diskLba, UINTN dataSize, UINT32 diskMediaID, bool executable) {
  EFI_PHYSICAL_ADDRESS buffer = 0;
  EFI_STATUS status = EFI_SUCCESS;

  // Loop through and get Block IO Protocol for input media ID, for entire disk. 
  // NOTE: This assumnes that the first block io found with logical partition false is the disk entirely itself.
  EFI_GUID bipGuid = EFI_BLOCK_IO_PROTOCOL_GUID;
  EFI_BLOCK_IO_PROTOCOL *bip = NULL;
  UINTN numHandles = 0;
  EFI_HANDLE *handleBuffer = NULL;
  
  status = bs->LocateHandleBuffer(ByProtocol, &bipGuid, NULL, &numHandles, &handleBuffer);
  if(EFI_ERROR(status)) {
    error(status, u"Could not locate Block IO Protocol handles!\r\n");
    return buffer;
  }

  BOOLEAN found = FALSE;
  UINTN i = 0;
  for (i = 0; i < numHandles; i++) {
    // Open Block IO Protocol for each handle
    status = bs->OpenProtocol(handleBuffer[i], &bipGuid, (VOID **)&bip, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if(EFI_ERROR(status)) {
      error(status, u"Could not open Block IO Protocol for handle %u!\r\n", i);
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
    error(status, u"Could not find Block IO Protocol for disk with ID: %u!\r\n", diskMediaID);
    return buffer;
  } 

  // Get Disk IO Protocol on the same device as block IO Protocol
  EFI_GUID dipGuid = EFI_DISK_IO_PROTOCOL_GUID;
  EFI_DISK_IO_PROTOCOL *dip = NULL;

  status = bs->OpenProtocol(handleBuffer[i], &dipGuid, (VOID **)&dip, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  if(EFI_ERROR(status)) {
    error(status, u"Could not open Disk IO Protocol on handle: %u!\r\n", i);
    goto cleanup;
  }

  // Allocate buffer for data
  UINTN pagesNeeded = (dataSize + (PAGE_SIZE-1)) / PAGE_SIZE; 
  if (executable) {
    status = bs->AllocatePages(AllocateAnyPages, EfiLoaderCode, pagesNeeded, &buffer);
  } else {
    status = bs->AllocatePages(AllocateAnyPages, EfiLoaderData, pagesNeeded, &buffer);
  }
  if(EFI_ERROR(status) || (VOID *)buffer == NULL) {
    error(status, u"Could not allocate buffer for disk data!\r\n");
    bs->CloseProtocol(handleBuffer[i], &dipGuid, image, NULL);
    goto cleanup;
  }

  // Use Disk IO Read to read into allocated buffer
  status = dip->ReadDisk(dip, diskMediaID, diskLba * bip->Media->BlockSize, dataSize, (VOID *)buffer);
  if(EFI_ERROR(status)) {
    error(status, u"Could not read disk LBAs into buffer!\r\n");
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
    error(status, u"Could not locate GOP!\r\n");
    return status;
  }

  // Overall Screen Loop
  while(true) {
    cout->ClearScreen(cout);

    printf(u"Graphics mode information:\r\n");

    //Get current GOP mode information
    status = gop->QueryMode(gop, gop->Mode->Mode, &modeInfoSize, &modeInfo);

    if(EFI_ERROR(status)) {
      error(status, u"Could not Query GOP Mode %u\r\n", gop->Mode->Mode);
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
    error(status, u"Could not locate GOP!\r\n");
    return status;
  }

  gop->QueryMode(gop, gop->Mode->Mode, &modeInfoSize, &modeInfo);

  // Use LocateHandleBuffer() to find all SPP and get a valid one.
  status = bs->LocateHandleBuffer(ByProtocol, &sppGuid, NULL, &numHandles, &handleBuffer);
  if (EFI_ERROR(status)) {
    error(status, u"\r\nCould not locate SPP handle buffer!\r\n");
  }

  cout->ClearScreen(cout);

  BOOLEAN foundMode = FALSE;
  // Open all SPP for each handle.
  for (UINTN i=0; i < numHandles; i++) {
    status = bs->OpenProtocol(handleBuffer[i], &sppGuid, (VOID **)&spp[i], image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if(EFI_ERROR(status)) {
      error(status, u"\r\nCould not Open SPP on handle!\r\n");
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

  if(!foundMode) error(0, u"\r\nCould not find any valid SPP mode!\r\n");  

  // Free memory pool allocated by LocateHandleBuffer()
  bs->FreePool(handleBuffer);

  // Use LocateHandleBuffer() to find all APP and get a valid one.
  numHandles = 0;
  handleBuffer = NULL;
  foundMode = FALSE;

  status = bs->LocateHandleBuffer(ByProtocol, &appGuid, NULL, &numHandles, &handleBuffer);
  if (EFI_ERROR(status)) {
    error(status, u"\r\nCould not locate APP handle buffer!\r\n");
  }

  // Open all APP for each handle.
  for (UINTN i=0; i < numHandles; i++) {
    status = bs->OpenProtocol(handleBuffer[i], &appGuid, (VOID **)&app[i], image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if(EFI_ERROR(status)) {
      error(status, u"\r\nCould not Open APP on handle!\r\n");
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

  if(!foundMode) error(0, u"\r\nCould not find any valid APP mode!\r\n");

  if(numProtocols == 0 ) {
    error(0, u"\r\nCould not find any S/APPs!\r\n");
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
    error(status, u"Could not open Loaded Image Protocol\r\n");
    return status;
  }

  // Get Simple File System Protocol for device handle for this loaded image, to open the root directory for the ESP.
  EFI_GUID sfspGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp = NULL;

  status = bs->OpenProtocol(lip->DeviceHandle, &sfspGuid, (VOID **)&sfsp, image, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  
  if(EFI_ERROR(status)) {
    error(status, u"Could not open Simple FileSystem Protocol\r\n");
    return status;
  }
  
  // Open root directory via OpenVolume()
  EFI_FILE_PROTOCOL *dirp = NULL;
  status = sfsp->OpenVolume(sfsp, &dirp);

  if(EFI_ERROR(status)) {
    error(status, u"Could not open volume for root directory\r\n");
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
              error(status, u"Could not open new directory %s\r\n", fileInfo.FileName);
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
              error(status, u"Could not allocate memory for file %s\r\n", fileInfo.FileName);
              goto cleanup;
          }

          // Open File
          EFI_FILE_PROTOCOL *file;
          status = dirp->Open(dirp, &file, fileInfo.FileName, EFI_FILE_MODE_READ, 0);

          if(EFI_ERROR(status)) {
              error(status, u"Could not open file %s\r\n", fileInfo.FileName);
              goto cleanup;
          }

          // Read file into buffer
          buffSize = fileInfo.FileSize;
          status = dirp->Read(file, &buffSize, buffer);
          if(EFI_ERROR(status)) {
              error(status, u"Could not read file %s into buffer.\r\n", fileInfo.FileName);
              goto cleanup;
          }

          if(buffSize != fileInfo.FileSize) {
            error(
              0,
              u"Could not read all file %s into buffer.\r\n"
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
    error(status, u"Could not open Loaded Image protocol\r\n");
    goto done;
  }

  EFI_GUID bipGuid = EFI_BLOCK_IO_PROTOCOL_GUID;
  EFI_BLOCK_IO_PROTOCOL *bip = NULL;
  status = bs->OpenProtocol(lip->DeviceHandle, &bipGuid, (VOID **)&bip, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if(EFI_ERROR(status)) {
    error(status, u"Could not open Block IO Protocol for this loaded image.\r\n");
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
    error(status, u"Could not get disk image media ID.\r\n");
    return status;
  }

  // Loop through and print all partition info found
  status = bs->LocateHandleBuffer(ByProtocol, &bipGuid, NULL, &numHandles, &handleBuffer);
  if(EFI_ERROR(status)) {
    error(status, u"Could not locate Block IO Protocols.\r\n");
    return status;
  }
  
  UINT32 lastMediaId = -1; // Keep track of currently opened Media info
  for(UINTN i = 0; i < numHandles; i++){
    status = bs->OpenProtocol(handleBuffer[i], &bipGuid, (VOID **)&bip, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if(EFI_ERROR(status)) {
      error(status, u"Could not open any Block IO protocol on handle %u\r\n", i);
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
        error(status, u"\r\nCould not open partition info protocol on handle %u.\r\n", i);
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
VOID *loadElf(VOID *elfBuffer, EFI_PHYSICAL_ADDRESS *kernelBuffer, UINTN *kernelSize) {
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
    error(0, u"ELF file is not a PIE file, only ET_DYN/0x03 type is allowed.\r\n");
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
  EFI_PHYSICAL_ADDRESS programBuffer = 0;
  UINTN pagesNeeded = (maxMemoryNeeded + (PAGE_SIZE - 1)) / PAGE_SIZE;

  // May want to switch this for alloc a kernel to use AllocateAddress to put the buffer start at a specific address e.g. in higher half memory
  status = bs->AllocatePages(AllocateAnyPages, EfiLoaderCode, pagesNeeded, &programBuffer);
  if(EFI_ERROR(status)) {
    error(status, u"Could not allocate memory for ELF program headers.\r\n");
    return NULL;
  }

  // Zero init buffer, to ensure 0 padding for all program sections
  memset((VOID *)programBuffer, 0, maxMemoryNeeded);

  // Fill in input params
  *kernelBuffer = programBuffer;
  *kernelSize = pagesNeeded * PAGE_SIZE;

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
VOID *loadPe(VOID *peBuffer, EFI_PHYSICAL_ADDRESS *kernelBuffer, UINTN *kernelSize) {
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
    error(0, u"PE file is not a 64-bit file, only AMD64/0x8664 type is allowed.\r\n");
    return NULL;
  }

  if(!(coffHeader->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
    error(0, u"File is not an executable.\r\n");
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
    error(0, u"File not a PE32+ file.\r\n");
    return NULL;
  }

  if(!(optHeader->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE)) {
    error(0, u"File not a PIE.\r\n");
    return NULL;
  }

  // Allocate buffer to load sections into
  EFI_STATUS status = 0;
  EFI_PHYSICAL_ADDRESS progBuffer = 0;
  UINTN pagesNeeded = (optHeader->SizeOfImage + (PAGE_SIZE-1)) / PAGE_SIZE;
  status = bs->AllocatePages(AllocateAnyPages, EfiLoaderCode, pagesNeeded, &progBuffer);
  if(EFI_ERROR(status)) {
    error(status, u"Could not allocate memory for PE file.\r\n");
    return NULL;
  }

  // Initialize buffer to 0, which should also take care of needing to 0-pad sections between raw data and virtual size
  memset((VOID *)progBuffer, 0, optHeader->SizeOfImage);

  *kernelBuffer = progBuffer;
  *kernelSize = pagesNeeded * PAGE_SIZE;

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
      if(*pos == '\0') break; 
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
    error(status, u"Could not get initial memory map size\r\n");
    return status;
  }

  // Allocate buffer for actual memory map size
  mMap->size += mMap->descriptorSize * 2; // Allocate enough space for an additional memory descriptor or 2 in the map due to this allocation itself.
  status = bs->AllocatePool(EfiLoaderData, mMap->size ,(VOID **)&mMap->map);
  if(EFI_ERROR(status)) {
    error(status, u"Could not allocate buffer for memory map size\r\n");
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
    error(status, u"Could not get UEFI memory map!\r\n");
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

  MemoryMapInfo mMap = {0}; // 1488th line of code?? hitler reference? :D
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

    // Pause pause if bottom of screeen reached
    if(cout->Mode->CursorRow >= textRows-2) {
      printf(u"Press any key to continue...\r\n");
      getKey();
      cout->ClearScreen(cout);
    }
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
// Get specific config table pointer by GUID
// =================
VOID *getConfigTableByGuid(EFI_GUID guid) {
  for(UINTN i = 0; i < st->NumberOfTableEntries; i++) {
    EFI_GUID vendorGuid = st->ConfigurationTable[i].VendorGuid;

    if(!memcmp(&vendorGuid, &guid, sizeof guid)) return st->ConfigurationTable[i].VendorTable;
  }

  return NULL; // Did not find config table
}

// =================
// Print ACPI Table header
// =================
void printAcpiTableHeader(ACPI_TABLE_HEADER header) {
  printf(
    u"Signature: %.4hhs\r\n"
    u"Length: %u\r\n"
    u"Revision: %#x\r\n"
    u"Checksum: %u\r\n"
    u"OEMID: %.6hhs\r\n"
    u"OEM Table ID: %.8hhs\r\n"
    u"OEM Revision: %#x\r\n"
    u"Creator ID: %#x\r\n"
    u"Creator Revision: %#x\r\n",
    &header.signature[0],
    (UINTN)header.length,
    (UINTN)header.revision,
    (UINTN)header.checksum,
    &header.OEMID[0],
    &header.OEMTableID[0],
    (UINTN)header.OEMRevision,
    (UINTN)header.creatorID,
    (UINTN)header.creatorRevision
  );
}

// =================
// Print configuration tables
// =================
EFI_STATUS printConfigTables(void) {
  cout->ClearScreen(cout);
  bs->CloseEvent(timerEvent);

  printf(u"Configuration Table GUIDs:\r\n");
  for(UINTN i = 0; i < st->NumberOfTableEntries; i++) {
    EFI_GUID guid = st->ConfigurationTable[i].VendorGuid;
    printGuid(guid);
    UINTN j = 0;

    // Print GUID name if available
    bool found = false;
    for (j = 0; j < ARR_SIZE(configTableGuidsAndStrings); j++) {
      if(!memcmp(&guid, &configTableGuidsAndStrings[j].guid, sizeof guid)) {
        found = true;
        break;
      }
    }
    printf(u"\r\n(%s)\r\n", found ? configTableGuidsAndStrings[j].string : u"Unknown GUID Value");
    // Pause every so often
    if(i > 0 && i % 6 == 0) getKey();
  }

  printf(u"\r\nPress any key to go back...\r\n");
  getKey();

  return EFI_SUCCESS;
}

// ================
// Print ACPI Tables
// ================
EFI_STATUS printAcpiTables(void) {
  cout->ClearScreen(cout);
  bs->CloseEvent(timerEvent);

  EFI_GUID acpiGuid = EFI_ACPI_TABLE_GUID;

  // Check for ACPI 2.0+ table
  VOID *rsdpPtr = getConfigTableByGuid(acpiGuid);
  bool acpi2 = false;
  if(!rsdpPtr) {
    // Check for ACPI 1.0 table as fallback
    acpiGuid = (EFI_GUID)ACPI_TABLE_GUID;
    rsdpPtr = getConfigTableByGuid(acpiGuid);
    if(!rsdpPtr) {
      error(0, u"Could not find ACPI Configuration Table\r\n");
      return 1;
    } else {
      printf(u"ACPI 1.0 Table found at %#x\r\n\r\n", rsdpPtr);
    }
  } else {
    printf(u"ACPI 2.0+ Table found at %#x\r\n\r\n", rsdpPtr);\
    acpi2 = true;
  }

  // Print RSDP
  UINT8 *rsdp = rsdpPtr; 
  if(acpi2) {
    printf(
      u"RSDP:\r\n"
      u"Signature: %.8hhs\r\n"
      u"Checksum: %u\r\n"
      u"OEMID: %.6hhs\r\n"
      u"RSDT Address: %x\r\n"
      u"Length: %u\r\n"
      u"XSDT Address: %x\r\n"
      u"Extended Checksum: %u\r\n",
      &rsdp[0],
      (UINTN)rsdp[8],
      &rsdp[9],
      *(UINT32 *)&rsdp[16],
      *(UINT32 *)&rsdp[20],
      *(UINT64 *)&rsdp[24],
      (UINTN)rsdp[32]
    );
  } else {
    printf(
      u"RSDP:\r\n"
      u"Signature: %.8hhs\r\n"
      u"Checksum: %u\r\n"
      u"OEMID: %.6hhs\r\n"
      u"RSDT Address: %x\r\n",
      &rsdp[0],
      (UINTN)rsdp[8],
      &rsdp[9],
      *(UINT32 *)&rsdp[16]
    );
  }
   
  printf(u"\r\nPress any key to print RSDT/XSDT...\r\n");
  getKey();

  ACPI_TABLE_HEADER *header = NULL;
  UINT64 xsdtAddress =  *(UINT64 *)&rsdp[24];
  if(acpi2) {
    // Print XSDT header
    header = (ACPI_TABLE_HEADER *)xsdtAddress;
    printAcpiTableHeader(*header);

    printf(u"\r\nPress any key to print XSDT Entries...\r\n");
    getKey();
    // Print XSDT Entries
    printf(u"Entries:\r\n");
    UINT64 *entry = (UINT64 *)((UINT8 *)header + sizeof *header); // Header = *= Size of header
    for (UINTN i = 0; i < (header->length - sizeof *header) / 8; i++) {
      ACPI_TABLE_HEADER *tableHeader = (ACPI_TABLE_HEADER *)entry[i];
      printf(
        u"%c%c%c%c\r\n",
        tableHeader->signature[0], tableHeader->signature[1], tableHeader->signature[2], tableHeader->signature[3] 
      );
      if (i > 0 && i % 23 == 0) getKey();
      // Print more than only signature
    }
  } else {
    // Print RSDT header & entries

  }

  // Print other tables from XSDT/RSDT entries

  printf(u"\r\nPress any key to go back...\r\n");
  getKey();
  return EFI_SUCCESS;
}

// =================
// ALlocate pages from available UEFI Memory Map
// =================
void *allocateMemoryMapPages(MemoryMapInfo *mMap, UINTN pages) {
  static void *nextPageAddr = NULL; // Next page/page range address to return to called
  static UINTN currDescriptor = 0; // Current descriptor number
  static UINTN remainingPages = 0;  // Remaining pages in current descriptor

  if (remainingPages < pages) {
    // Not enough remaining pages in current descriptor, find next available one
    UINTN i = 0;
    for (i = currDescriptor + 1; i < mMap->size / mMap->descriptorSize; i++) {
      EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)mMap->map + (i * mMap->descriptorSize));
      if (desc->Type == EfiConventionalMemory && desc->NumberOfPages >= pages) {
        // Found enough memory to use at this descriptor, use it
        currDescriptor = i;
        remainingPages = desc->NumberOfPages - pages;
        nextPageAddr = (void *)(desc->PhysicalStart + (pages * PAGE_SIZE));
        return (void *)desc->PhysicalStart;
      }
    }
    if(i >= mMap->size / mMap->descriptorSize ) {
      // Ran out of descriptiros to check in memory map
      error(0, u"Could not find any memory to allocate pages for.\r\n");
      return NULL;
    }
  }

  // Else we have at least enough pags for this allocation, return the current spot in memory map
  remainingPages -= pages;
  void *page = nextPageAddr;
  nextPageAddr = (void *)((UINT8 *)page + (pages * PAGE_SIZE));
  return page;
}

// =================
// Map a virtual addres to a physical address
// =================
void mapPage(UINTN physAddr, UINTN virAddr, MemoryMapInfo *mMap) {
  int flags = PRESENT | RW | USER; // 0b111

  UINTN pml4Idx = ((virAddr) >> 39) & 0x1FF; // 0-511
  UINTN pdptIdx = ((virAddr) >> 30) & 0x1FF; // 0-511
  UINTN pdtIdx  = ((virAddr) >> 21) & 0x1FF; // 0-511
  UINTN ptIdx   = ((virAddr) >> 12) & 0x1FF; // 0-511

  // Make sure pdpt exists, if not, then allocate it
  if(!(pml4->entries[pml4Idx] & PRESENT)) {
    void *pdptAddr = allocateMemoryMapPages(mMap, 1);

    memset(pdptAddr, 0, sizeof(pageTable));
    pml4->entries[pml4Idx] = (UINTN)pdptAddr | flags;
  }

  // Make sure pdt exists, if not, allocate
  pageTable *pdpt = (pageTable *)(pml4->entries[pml4Idx] & PHYSICAL_PAGE_ADDR_MASK);
  if(!(pdpt->entries[pdptIdx] & PRESENT)) {
    void *pdtAddr = allocateMemoryMapPages(mMap, 1);
    
    memset(pdtAddr, 0, sizeof(pageTable));
    pdpt->entries[pdptIdx] = (UINTN)pdtAddr | flags;
  }

  // Make sure pt exists, if not allocate
  pageTable *pdt = (pageTable *)(pdpt->entries[pdptIdx] & PHYSICAL_PAGE_ADDR_MASK);
  if(!(pdt->entries[pdtIdx] & PRESENT)) {
    void *ptAddr = allocateMemoryMapPages(mMap, 1);
    
    memset(ptAddr, 0, sizeof(pageTable));
    pdt->entries[pdtIdx] = (UINTN)ptAddr | flags;
  }

  // Map new page if not present
  pageTable *pt = (pageTable *)(pdt->entries[pdtIdx] & PHYSICAL_PAGE_ADDR_MASK);
  if(!(pt->entries[ptIdx] & PRESENT)) pt->entries[ptIdx] = (physAddr & PHYSICAL_PAGE_ADDR_MASK) | flags;
}

// =================
// Unmap a page/virtAddr
// =================
void unmapPage(UINTN virAddr) {
  UINTN pml4Idx = ((virAddr) >> 39) & 0x1FF; // 0-511
  UINTN pdptIdx = ((virAddr) >> 30) & 0x1FF; // 0-511
  UINTN pdtIdx  = ((virAddr) >> 21) & 0x1FF; // 0-511
  UINTN ptIdx   = ((virAddr) >> 12) & 0x1FF; // 0-511

  pageTable *pdpt = (pageTable *)(pml4->entries[pml4Idx] & PHYSICAL_PAGE_ADDR_MASK);
  pageTable *pdt = (pageTable *)(pdpt->entries[pdptIdx] & PHYSICAL_PAGE_ADDR_MASK);
  pageTable *pt = (pageTable *)(pdt->entries[pdtIdx] & PHYSICAL_PAGE_ADDR_MASK);

  pt->entries[ptIdx] = 0; /// Wipe page in page table to unmap phys addr there

  // Flush the TLB cache for this cache
  __asm__ __volatile__("invlpg (%0)\n" : : "r"(virAddr));

}

// =================
// Identity map a page of memory, virtual = phys addr
// =================
void identityMapPage(UINTN address, MemoryMapInfo *mMap) {
  mapPage(address, address, mMap);
}

// =================
// Initialize new paging setup by identity mapping all available memory from EFI memory map
// =================
void identityMapEfiMemMap(MemoryMapInfo *mMap) {
  UINTN descCount = mMap->size / mMap->descriptorSize;
  for (UINTN i = 0; i < descCount; i++) {
    EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)mMap->map + (i * mMap->descriptorSize));

    for (UINTN j = 0; j < desc->NumberOfPages; j++) identityMapPage(desc->PhysicalStart + (j * PAGE_SIZE), mMap);
  }
}

// =================
// Identity map runtime memory desc only, to use with runtime services setVirtualAddressMap();
// =================
void setRuntimeAddrMap(MemoryMapInfo *mMap){
  // First get amount of memory to alloc for runtime memory map
  UINTN runtimeDescriptors = 0;
  UINTN descCount = mMap->size / mMap->descriptorSize;
  for (UINTN i = 0; i < descCount; i++) {
    EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)mMap->map + (i * mMap->descriptorSize));
    
    if (desc->Attribute & EFI_MEMORY_RUNTIME) runtimeDescriptors++;
  }

  // Alloc memory for runtime mem map
  UINTN runtimeMemMapPages = (runtimeDescriptors * mMap->descriptorSize) + ((PAGE_SIZE-1) / PAGE_SIZE);
  EFI_MEMORY_DESCRIPTOR *runtimeMemMap = allocateMemoryMapPages(mMap, runtimeMemMapPages);
  if (!runtimeMemMap) {
    error(0, u"Could not allocate runtime descriptors memory map.\r\n");
    return;
  }

  // Identity map all runtime descs in each desc
  UINTN runtimeMemMapSize = runtimeMemMapPages * PAGE_SIZE;
  memset(runtimeMemMap, 0, runtimeMemMapSize);

  // Set all runtime descriptors in new runtime memory map, and identity map them
  UINTN currentRuntimeDesc = 0;
  for (UINTN i = 0; i < descCount; i++) {
    EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)mMap->map + (i * mMap->descriptorSize));
    
    if (desc->Attribute & EFI_MEMORY_RUNTIME) {
      EFI_MEMORY_DESCRIPTOR *runtimeDesc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)runtimeMemMap + (currentRuntimeDesc * mMap->descriptorSize));

      memcpy(runtimeDesc, desc, mMap->descriptorSize);
      runtimeDesc->VirtualStart = runtimeDesc->PhysicalStart;
      currentRuntimeDesc++;
    }
  }

  // Set new virtual addresses for runtime memory via SetVirtualAddressMap()
  EFI_STATUS status = rs->SetVirtualAddressMap(runtimeMemMapSize, mMap->descriptorSize, mMap->descriptorVersion, runtimeMemMap);
  if(EFI_ERROR(status)) {
    error(0, u"SetVirtualAddressMap()\r\n");
  }
}

// =================
// Print global EFI vars
// =================
EFI_STATUS printEfiGlbVars(void) {
  cout->ClearScreen(cout);

  bs->CloseEvent(timerEvent);

  UINTN varNameSize = 0;
  CHAR16 *varNameBuffer = 0;
  EFI_GUID vendorGuid = {0};
  EFI_STATUS status = EFI_SUCCESS;

  varNameSize = 2;
  status = bs->AllocatePool(EfiLoaderData, varNameSize, (VOID **)&varNameBuffer);
  if(EFI_ERROR(status)) {
    error(status, u"Could not allocate 2 bytes...\r\n");
    return status;
  }

  // Set var name to point to initial single null byte, to start off call to get list of var names
  *varNameBuffer = u'\0';

  status = rs->GetNextVariableName(&varNameSize, varNameBuffer, &vendorGuid);
  while(status != EFI_NOT_FOUND) { // End of list
    if(status == EFI_BUFFER_TOO_SMALL) {
      // Reallocate larger buffer for var name
      CHAR16 *tempBuffer = NULL;
      status = bs->AllocatePool(EfiLoaderData, varNameSize, (VOID **)&tempBuffer);
      if(EFI_ERROR(status)) {
        error(status, u"Could not allocate %u bytes of memory for next variable name. \r\n", varNameSize);
        return status;
      }

      strcpy_u16(tempBuffer, varNameBuffer); // copy old buffer tp new buffer
      bs->FreePool(varNameBuffer); // Free old buffer
      varNameBuffer = tempBuffer; // Set new buffer
      //tempSize = varNameSize; // Set new buffer size

      status = rs->GetNextVariableName(&varNameSize, varNameBuffer, &vendorGuid);
      continue;
    }

    // Print var name
    printf(u"%.*s\r\n", varNameSize, varNameBuffer);

    if(cout->Mode->CursorRow >= textRows-2) {
      printf(u"Press any key to continue...\r\n");
      getKey();
      cout->ClearScreen(cout);
    }

    status = rs->GetNextVariableName(&varNameSize, varNameBuffer, &vendorGuid);
  }
  
  // Free buffer when done
  bs->FreePool(varNameBuffer);

  printf(u"\r\nPress any key to go back...\r\n");
  getKey();
  return EFI_SUCCESS;
}

// =================
// Print & Change Boot Variables
// =================
EFI_STATUS changeBootVars(void) {
  cout->ClearScreen(cout);

  bs->CloseEvent(timerEvent);

  // Get Device Path to Text protocol to print Load Option device/file paths
  EFI_STATUS status = EFI_SUCCESS;
  EFI_GUID dpttpGuid =  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *dpttp;
  status = bs->LocateProtocol(&dpttpGuid, NULL, (VOID **)&dpttp);
  if(EFI_ERROR(status)) {
    error(status, u"Could not locate Device Path To Text Protocol!\r\n");
    return status;
  }

  UINT32 bootOrderAttr = 0;
  // Overall screen loop
  while(true) {
    cout->ClearScreen(cout);
    UINTN varNameSize = 0;
    CHAR16 *varNameBuffer = 0;
    EFI_GUID vendorGuid = {0};  

    varNameSize = 2;
    status = bs->AllocatePool(EfiLoaderData, varNameSize, (VOID **)&varNameBuffer);
    if(EFI_ERROR(status)) {
      error(status, u"Could not allocate 2 bytes...\r\n");
      return status;
    }

    // Set var name to point to initial single null byte, to start off call to get list of var names
    *varNameBuffer = u'\0';

    status = rs->GetNextVariableName(&varNameSize, varNameBuffer, &vendorGuid);
    while(status != EFI_NOT_FOUND) { // End of list
      if(status == EFI_BUFFER_TOO_SMALL) {
        // Reallocate larger buffer for var name
        CHAR16 *tempBuffer = NULL;
        status = bs->AllocatePool(EfiLoaderData, varNameSize, (VOID **)&tempBuffer);
        if(EFI_ERROR(status)) {
          error(status, u"Could not allocate %u bytes of memory for next variable name. \r\n", varNameSize);
          return status;
        }

        strcpy_u16(tempBuffer, varNameBuffer); // copy old buffer tp new buffer
        bs->FreePool(varNameBuffer); // Free old buffer
        varNameBuffer = tempBuffer; // Set new buffer
        //tempSize = varNameSize; // Set new buffer size

        status = rs->GetNextVariableName(&varNameSize, varNameBuffer, &vendorGuid);
        continue;
      }

      // Print var name and val
      if(!memcmp(varNameBuffer, u"Boot", 8)) {
        printf(u"\r\n%.*s: ", varNameSize, varNameBuffer);

        // Get var value
        UINT32 attributes = 0;
        UINTN dataSize = 0;
        VOID *data = 0;
        // Call first with 0 data size to get actual amount needed
        rs->GetVariable(varNameBuffer, &vendorGuid, &attributes, &dataSize, NULL);

        status = bs->AllocatePool(EfiLoaderData, dataSize, (VOID **)&data);
        if(EFI_ERROR(status)) {
          error(status, u"Could not allocate %u bytes of memory for GetVariable().\r\n", dataSize);
          goto cleanup;
        }

        // Get actual data now with correct size
        rs->GetVariable(varNameBuffer, &vendorGuid, &attributes, &dataSize, data);
        if(dataSize == 0) goto next;

        if(!memcmp(varNameBuffer, u"BootOrder", 18)) {
          bootOrderAttr = attributes; // Use of sets new BootOrder value;
          // Array of UINT16 values
          UINT16 *p = data;
          for(UINTN i = 0; i < dataSize / 2; i++) printf(u"%#.4x,", *p++);
          printf(u"\r\n");
          goto next;
        }

        if(!memcmp(varNameBuffer, u"BootOptionSupport", 34)) {
          // Single UINT16 value
          UINT32 *p = data;
          printf(u"%#.8x\r\n", *p);
          goto next;
        }

        if(!memcmp(varNameBuffer, u"BootNext", 18) || !memcmp(varNameBuffer, u"BootCurrent", 22)) {
          // Single UINT16 Value
          UINT16 *p = data;
          printf(u"%#.4hx\r\n", *p);
          goto next;
        }

        if(isHexDigitC16(varNameBuffer[4]) && varNameSize == 18) {
          // Boot#### load option
          EFI_LOAD_OPTION *loadOption = (EFI_LOAD_OPTION *)data;
          CHAR16 *desc = (CHAR16 *)((UINT8 *)data + sizeof(UINT32) + sizeof(UINT16));
          printf(u"%s\r\n", desc);

          CHAR16 *p = desc;
          UINTN strlen = 0;
          while(p[strlen]) strlen++;
          strlen++; // Skip null byte

          EFI_DEVICE_PATH_PROTOCOL *filePathList = (EFI_DEVICE_PATH_PROTOCOL *)(desc + strlen);

          CHAR16 *devicePathText = dpttp->ConvertDevicePathToText(filePathList, FALSE, FALSE);
          printf(u"Device Path: %s\r\n", devicePathText ? devicePathText : u"(null)");
          
          UINT8 *optionalData = (UINT8 *)filePathList + loadOption->FilePathListLength;
          UINTN optionalDataSize = dataSize - (optionalData - (UINT8 *)data);
          if(optionalDataSize > 0) {
            printf(u"Optional Data: 0x");
            for(UINTN i = 0; i < optionalDataSize; i++) printf(u"%.2hhx", optionalData[i]);
            printf(u"\r\n");
          }

          goto next;
        }

        printf(u"\r\n");
        
        next:
        status = bs->FreePool(data);

      };

      if(cout->Mode->CursorRow >= textRows-2) {
        printf(u"Press any key to continue...\r\n");
        getKey();
        cout->ClearScreen(cout);
      }

      status = rs->GetNextVariableName(&varNameSize, varNameBuffer, &vendorGuid);
    }

    // Allow user to change values
    printf(u"Press '1' to change BootOrder, '2' to change BootNext, or other to go back...\r\n");
    EFI_INPUT_KEY key = getKey();
    if(key.UnicodeChar == u'1') {
      // Change BootOrder - set new array of UINT16 values
      #define MAX_BOOT_OPTIONS 10

      UINT16 optionArray[MAX_BOOT_OPTIONS] = {0};
      UINTN newOption = 0;
      UINT16 numOptions = 0;

      for(UINTN i = 0; i < MAX_BOOT_OPTIONS; i++) {
        printf(u"\r\nBoot Option %u (0000-FFFF): ", i+1);
        if(!getHex(&newOption)) break; // Stop processing
        optionArray[numOptions++] = newOption;
      }

      EFI_GUID guid = EFI_GLOBAL_VARIABLE_GUID;
      
      status = rs->SetVariable(u"BootOrder", &guid, bootOrderAttr, numOptions*2, optionArray);

      if(EFI_ERROR(status)) {
        error(status, u"Could not set new value for BootOrder\r\n");
      }

    } else if (key.UnicodeChar == u'2') {
      // Change BootNext value - set new UINT16
      printf(u"\r\nBootNext Value (0000-FFFF): ");
      UINTN value = 0;
      if(!getHex(&value)) {
        EFI_GUID guid = EFI_GLOBAL_VARIABLE_GUID;
        UINT32 attr = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
        status = rs->SetVariable(u"BootNext", &guid, attr, 2, &value);

        if(EFI_ERROR(status)) {
          error(status, u"Could not set new value for BootNext.\r\n");
        }
      }
    } else {
      bs->FreePool(varNameBuffer);
      break;
    }

    cleanup:
    // Free buffer when done
    bs->FreePool(varNameBuffer);
  }
  return EFI_SUCCESS;
}

// =================
// Load Kernel
// =================
EFI_STATUS loadKernel(void) {

  EFI_PHYSICAL_ADDRESS *diskBuffer = NULL;
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
    error(0, u"Could not find or read %s from data partition to buffer.\r\n", fileName);
    goto exit;
  }

  // Search for file based on file name
  strPos = substr(fileBuffer, "kernel");
  if(!strPos) {
    error(0, u"Could not find kernel file in data partition.\r\n");
    goto cleanup;
  }

  printf(u"KERNEL INFORMATION:\r\n");

  // Parse data from TEST.TXT file to get disk LBA and file size
  strPos = substr(fileBuffer, "FILE_SIZE=");
  if(!strPos) {
    error(0, u"Could not find file size for file %s.\r\n", fileName);
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
    error(0, u"Could not find disk lba value from buffer %s.\r\n", fileName);
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
    error(status, u"Could not find or get MediaID value for disk image");
    bs->FreePool(fileBuffer);
    goto exit;
  }

  // Read disk lbas for file into buffer
  diskBuffer = (VOID *)readDiskLbasToBuffer(diskLba, fileSize, imageMediaID, true);
  if(!diskBuffer) {
    error(0, u"Could not find or read data partition file to buffer.\r\n");
    bs->FreePool(diskBuffer); // Free memory allocated for disk LBA buffer
    goto exit;
  }

  // Get GOP info for kernel params
  EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;

  status = bs->LocateProtocol(&gopGuid, NULL, (VOID **)&gop);
  if(EFI_ERROR(status)) {
    error(status, u"Could not locate GOP for kernel parameters.\r\n");
    return status;
  }
  
  // Initialize Kernel params
  KernelParameters kparams = {0}; // Defined in efilib.h
  
  kparams.gopMode = *gop->Mode; // Copy GOP mode info to kernel params;
  EntryPoint entryPoint = NULL;

  // Load Kernel File depending on format (initial header bytes)
  UINT8 *hdr = (UINT8 *)diskBuffer;
  printf(u"File Format: ");
  printf(u"Header bytes: [%x][%x][%x][%x]\r\n", 
    hdr[0], 
    hdr[1], 
    hdr[2], 
    hdr[3]
  );

  EFI_PHYSICAL_ADDRESS kernelBuffer = 0;
  UINTN kernelSize = 0;
  
  if(!memcmp(hdr, (UINT8[4]){0x7F, 'E', 'L', 'F'}, 4)) {
    *(void **)&entryPoint = loadElf(diskBuffer, &kernelBuffer, &kernelSize);

  } else if(!memcmp(hdr, (UINT8[2]){'M', 'Z'}, 2)) {
    *(void **)&entryPoint = loadPe(diskBuffer, &kernelBuffer, &kernelSize);

  } else {
    printf(u"No header bytes, assuming this is a flat binary file.\r\n");
    // Flat binary executable code assumed to start at the beginning of the loaded buffer
    *(void **)&entryPoint = diskBuffer;
    kernelBuffer = (EFI_PHYSICAL_ADDRESS)diskBuffer;
    kernelSize = fileSize;
  }

  // Get new higher address kernel entry point to use
  UINTN entryOffset = (UINTN)entryPoint - kernelBuffer;
  EntryPoint higherEntry = (EntryPoint)(KERNEL_START_ADDR + entryOffset);

  // Print info about kernel
  printf(
    u"\r\nOriginal Kernel Address: %x, Size: %u\r\n"
    u"Entry Point: %x, Higher Entry Point: %x\r\n",
    kernelBuffer ,kernelSize, (UINTN)entryPoint, higherEntry
  );

  if(!entryPoint) {
    bs->FreePages(kernelBuffer, kernelSize / PAGE_SIZE);
    goto cleanup;
  }

  printf(u"Press any key to load kernel...\r\n");
  getKey();

  // Close timer event so that it does not continue to fire off
  bs->CloseEvent(timerEvent);

  cout->ClearScreen(cout);

  status = bs->AllocatePool(EfiLoaderData, sizeof(MemoryMapInfo), (VOID **)&kparams.mMap);
  if(EFI_ERROR(status)) {
    error(status, u"Could not allocate MemoryMapInfo struct\r\n");
    goto cleanup;
  }

  // Get Memory Map
  if(EFI_ERROR(getMemoryMap(kparams.mMap))) goto cleanup;

  UINTN retries = 0;
  const UINTN MAX_RETRIES = 5;
  while (EFI_ERROR(bs->ExitBootServices(image, kparams.mMap->key)) && retries < MAX_RETRIES) {
    bs->FreePool(kparams.mMap->map);
    if (EFI_ERROR(getMemoryMap(kparams.mMap))) goto cleanup;
    retries++;
  }
  if (retries == MAX_RETRIES) {
    error(0, u"Could not exit Boot Services!\r\n");
    goto cleanup;
  }

  // Set rest of kernel params
  kparams.RuntimeServices = rs;
  kparams.NumberOfTableEntries = st->NumberOfTableEntries;
  kparams.ConfigurationTable = st->ConfigurationTable;

  // Setup new level4 paging & page tables
  pml4 = allocateMemoryMapPages(kparams.mMap, 1);
  memset(pml4, 0, sizeof *pml4);

  // Initialize new paging setup by identity mapping all available memory
  identityMapEfiMemMap(kparams.mMap);

  // Identity map runtime services memory & set new addres map
  setRuntimeAddrMap(kparams.mMap);

  // Remap Kernel to higher address
  for (UINTN i = 0; i < (kernelSize + (PAGE_SIZE-1)) / PAGE_SIZE; i++) {
    mapPage(kernelBuffer + (i*PAGE_SIZE), KERNEL_START_ADDR + (i*PAGE_SIZE) , kparams.mMap);
  }

  // NOTE: Remap kparams to higher address?
   
  // Identity map framebuffer
  for (UINTN i = 0; i < (kparams.gopMode.FrameBufferSize + (PAGE_SIZE-1)) / PAGE_SIZE; i++) {
    identityMapPage(kparams.gopMode.FrameBufferBase + (i * PAGE_SIZE), kparams.mMap);
  }
  
  // Identity map new stack for kernel
  const UINTN STACK_PAGES = 16;
  void *kernelStack = allocateMemoryMapPages(kparams.mMap, STACK_PAGES); // 64KiB stack
  uint32_t stackSize = STACK_PAGES * PAGE_SIZE;
  memset(kernelStack, 0, stackSize);

  for (UINTN i=0; i < STACK_PAGES; i++) {
    identityMapPage((UINTN)kernelStack + (i*PAGE_SIZE), kparams.mMap);
  }

  // Set up new GDT ad TSS
  TSS tss = {.io_map_base = sizeof(TSS)};
  UINTN tssAddress = (UINTN)&tss;
  
  GDT gdt = {
    .null.value =         0x0000000000000000, 
    .kernelCode64.value = 0x00AF9A000000FFFF, 
    .kernelData64.value = 0x00CF92000000FFFF,
    .userCode64.value =   0x00AFFA000000FFFF, 
    .userData64.value =   0x00CFF2000000FFFF, 
    .kernelCode32.value = 0x00CF9A000000FFFF, 
    .kernelData32.value = 0x00CF92000000FFFF, 
    .userCode32.value =   0x00CFFA000000FFFF, 
    .userData32.value =   0x00CFF2000000FFFF,
    .tss = {
      .descriptor = {
        .limit_15_0 = sizeof tss -1,
        .base_15_0 = tssAddress & 0xFFFF,
        .base_23_16 = (tssAddress >> 16) & 0xFF,
        .type = 9, //0b1001 64bit TSS 
        .p = 1, // Present
        .base31_24 = (tssAddress >> 24) & 0xFF,
      },
      .base63_32 = (tssAddress >> 32) & 0xFFFFFFFF,
    }
  };

  DescriptorRegister gdtr = {.limit = sizeof gdt - 1, .base = (UINT64)&gdt};

  KernelParameters *kparamsPtr = &kparams; // Get pointer to kernel fpr RCX as first parameter for x86_64 MS ABI

  // Set new page tables (CR3 = PML4) and GDT (lgdt && ltr), and call entry point with params
  __asm__ __volatile__(
    "cli\n"                   // Clear interrupts before setting new page tables
    "movq %[pml4]0, %%CR3\n"  // Load new page tables
    "lgdt %[gdt]\n"           // Load new GDT from GDTR register
    "ltr %[tss]\n"            // Load new task register with new TSS value

    // Jump to new code segment in GDT (offset in GDT of 64 bit kernel/system code segment)
    "pushq $0x8\n"
    "leaq 1f(%%RIP), %%RAX\n"
    "pushq %%RAX\n"
    "lretq\n"

    // Executing code with new Code secment now, set up remaining segment registers
    "1:\n"
    "movq $0x10, %%RAX\n" // Data segment to use (64 bit kernel data segment, offset in GDT)
    "movq %%RAX, %%DS\n"  // Data segment
    "movq %%RAX, %%ES\n"  // Extra segment
    "movq %%RAX, %%FS\n"  // Extra segment (2), these also have different uses in Long Mode
    "movq %%RAX, %%GS\n"  // Extra segment (3), these also have different uses in Long Mode
    "movq %%RAX, %%SS\n"  // Stack segment

    // Set new stack value to use (for SP/stack pointer, etc.)
    "movq %[stack], %%RSP\n"

    // Call new entry point in higher memory
    "callq *%[entry]\n" // First param is kparamsPtr in RCX
  :
  : [pml4]"r"(pml4), [gdt]"m"(gdtr), [tss]"r"((UINT16)0x48),
    [stack]"gm"((UINTN)kernelStack + (STACK_PAGES * PAGE_SIZE)),
    [entry]"b"(higherEntry), "c"(kparamsPtr)
  : "rax", "memory"
  );

  // Test calling PE, ELF, and flat bin kernels/entry points

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