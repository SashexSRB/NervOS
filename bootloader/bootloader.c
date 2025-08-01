#include "bootloader.h"
// =================
// EFI Image Entry Point
// =================
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  // Global vars init
  initGlbVars(ImageHandle, SystemTable);
  
  // Reset console output
  cout->Reset(cout, false);
  cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));

  // Disable watchdog timer
  bs->SetWatchdogTimer(0, 0x10000, 0, NULL);

  EFI_FILE_PROTOCOL *root = espRootDir();
  if(root) {
    EFI_STATUS status = EFI_SUCCESS;
    CHAR16 *path = u"\\EFI\\BOOT\\INSTALL.DAT";
    EFI_FILE_PROTOCOL *file = NULL;
    status = root->Open(root, &file, path, EFI_FILE_MODE_READ, 0);
    autoloadKernel = !EFI_ERROR(status);
    if(file) file->Close(file);
    if(root) root->Close(root);
  }

  if(autoloadKernel) loadKernel(); // Load kernel, should not return!

  const CHAR16 *menuChoices[] = {
    u"Load Kernel",
    u"Set Text Mode",
    u"Set Graphics Mode",
    u"Test Mouse",
    u"Read ESP Files",
    u"Print Block IO Partitions",
    u"Print Memory Map",
    u"Print Configuration Tables",
    u"Print ACPI Tables",
    u"Print Global EFI Variables",
    u"Change Boot Variables",
    u"Write Disk Image To Other Disk",
  };

  EFI_STATUS (*menuFuncs[])(void) = {
    loadKernel,
    setTextMode,
    setGraphicsMode,
    testMouse,
    readEspFiles,
    printBlockIoPartitions,
    printMemoryMap,
    printConfigTables,
    printAcpiTables,
    printEfiGlbVars,
    changeBootVars,
    writeToAnotherDisk,
  };

  EFI_STATUS status;
  UINTN handleCount = 0;
  EFI_HANDLE *handleBuffer = NULL;
  UINTN handleIdx = 0;

  status = bs->LocateHandleBuffer(AllHandles, NULL, NULL, &handleCount, &handleBuffer);
  if(!EFI_ERROR(status)) {
    for(handleIdx = 0; handleIdx < handleCount; handleIdx++) {
      status = bs->ConnectController(handleBuffer[handleIdx], NULL, NULL, TRUE);
    }
    bs->FreePool(handleBuffer);
  }

  // Screen loop
  bool running = true;
  while(running) {
    cout->ClearScreen(cout);
    // Get current text mode ColsxRows values
    UINTN cols = 0, rows = 0;
    cout->QueryMode(cout, cout->Mode->Mode, &cols, &rows);
    // Set global rows/cols values
    textRows = rows, 
    textCols = cols;

    // Timer context will be text mode screen boundds
    typedef struct {
      UINT32 rows, cols;
    } TimerContext;
    TimerContext context = {0};
    context.rows = rows;
    context.cols = cols;

    // Close Timer Event
    bs->CloseEvent(timerEvent);

    // Create timer event, to print datetime on screen every second.
    bs->CreateEvent(
      EVT_TIMER | EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      printDateTime,
      (VOID *)&context,
      &timerEvent
    );

    // Set timer for the timer event to run every 100ns (1s)
    bs->SetTimer(timerEvent, TimerPeriodic, 10000000);

    // Print keybinds at the bottom of the screen
    cout->SetCursorPosition(cout, 0, rows-3);
    printf(
      u"Up/Down Arrow = Move\r\n"
      u"Enter = Select\r\n"
      u"Escape = Shutdown"
    );

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
          // de-highlight current row,
          cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
          printf(u"%s\r", menuChoices[currentRow]);
          
          if(currentRow-1 >= minRow) currentRow--; // Go up one row
          else currentRow = maxRow; // Wrap around to bottom of the menu

          cout->SetCursorPosition(cout, 0, currentRow);
          cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
          printf(u"%s\r", menuChoices[currentRow]);
          cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
        break;
        case SCANCODE_DOWN_ARROW:
          // de-highlight current row,
          cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
          printf(u"%s\r", menuChoices[currentRow]);

          if(currentRow+1 <= maxRow) currentRow++; // Go down one row
          else currentRow = minRow; // Wrap around to top of the menu

          cout->SetCursorPosition(cout, 0, currentRow);
          cout->SetAttribute(cout, EFI_TEXT_ATTR(HL_FG_COLOR, HL_BG_COLOR));
          printf(u"%s\r", menuChoices[currentRow]);
          cout->SetAttribute(cout, EFI_TEXT_ATTR(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR));
          
        break;
        case SCANCODE_ESC:
          // Close Timer Event
          bs->CloseEvent(timerEvent);
          // Escape, poweroff
          rs->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          // !NOTE!: This should not return, sys should poweroff.
        break;
        default:
          if(key.UnicodeChar == u'\r') {
            EFI_STATUS returnStatus = menuFuncs[currentRow]();
            if(EFI_ERROR(returnStatus)) {
              error(returnStatus, u"\r\nPress any key to go back...");
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
