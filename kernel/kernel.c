#include "../bootloader/efilib.h"
#include "nervimage.h"
#include <stdint.h>

// Sample kernel. Draws the NERV Coat of Arms
__attribute__((section(".kernel")))void EFIAPI kmain(KernelParameters params) {
  // Grab FrameBuffer and GOP info.
  UINT32 *fb = (UINT32 *)params.gopMode.FrameBufferBase;
  UINT32 screenWidth = params.gopMode.Info->HorizontalResolution;
  UINT32 screenHeight = params.gopMode.Info->VerticalResolution;
  UINT32 xres = params.gopMode.Info->PixelsPerScanLine;

  UINT32 imgWidth = 488;
  UINT32 imgHeight = 600;

  UINT32 x_offset = (screenWidth - imgWidth) / 2;
  UINT32 y_offset = (screenHeight - imgHeight) / 2;

  for (UINT32 y = 0; y < imgHeight; y++) {
    for (UINT32 x = 0; x < imgWidth; x++) {
      UINT32 img_idx = (y * imgWidth + x) * 4;

      UINT8 r = NERV_raw[img_idx + 0];
      UINT8 g = NERV_raw[img_idx + 1];
      UINT8 b = NERV_raw[img_idx + 2];
      UINT8 a = NERV_raw[img_idx + 3];

      UINT32 color = (a << 24) | (r << 16) | (g << 8) | b;

      UINT32 fb_x = x + x_offset;
      UINT32 fb_y = y + y_offset;
      fb[fb_y * xres + fb_x] = color;
    }
  }

  // Test runtime services by waiting for few seconds and then shutting down
  EFI_TIME oldTime, newTime;
  EFI_TIME_CAPABILITIES timeCap;
  params.RuntimeServices->GetTime(&newTime, &timeCap);
  UINTN i = 0;
  while (i < 5) {
    params.RuntimeServices->GetTime(&newTime, &timeCap);
    if (oldTime.Second != newTime.Second) {
      i++;
      oldTime.Second = newTime.Second;
    }
  }

  params.RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

  // Should not return after shutting down
  __builtin_unreachable();
}

