#include "../bootloader/efi.h"
#include "../bootloader/efilib.h"
#include <stdint.h>

// Sample kernel

__attribute__((section(".kernel"), aligned(0x1000))) void EFIAPI kmain(KernelParameters *params) {
  // Grab FrameBuffer and GOP info.
  UINT32 *fb = (UINT32 *)params->gopMode.FrameBufferBase;
  UINT32 xres = params->gopMode.Info->PixelsPerScanLine;
  UINT32 yres = params->gopMode.Info->VerticalResolution;
  // Clean screen to a color
  for(UINT32 y = 0; y < yres; y++) {
    for(UINT32 x = 0; x < xres; x++) {
      fb[y * xres + x] = 0xFFFF0000; // Red color
    }
  }

  for(UINT32 y = 0; y < yres/5; y++) {
    for(UINT32 x = 0; x < xres/5; x++) {
      fb[y * xres + x] = 0xFFFF2222; // Darker Red color
    }
  }

   // Test runtime services by waiting for few seconds and then shutting down
  EFI_TIME oldTime, newTime;
  EFI_TIME_CAPABILITIES timeCap;
  params->RuntimeServices->GetTime(&newTime, &timeCap);
  UINTN i = 0;
  while (i < 5) {
    params->RuntimeServices->GetTime(&newTime, &timeCap);
    if (oldTime.Second != newTime.Second) {
      i++;
      oldTime.Second = newTime.Second;
    }
  }

  params->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

  // Infinite loop, do NOT return back to UEFI
  // This is in case hardware doesn't fully shut off. Can still use power button though
  while(true) {
    __asm__ __volatile__ ("cli; hlt");
  }

  // Should not return after shutting down
  __builtin_unreachable();
}
