#include "../bootloader/efi.h"
#include <stdint.h>

typedef struct {
  void *memoryMap;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE gopMode;
} KernelParameters;

// Sample kernel
void EFIAPI kmain(KernelParameters params) {
  // Grab FrameBuffer and GOP info.
  UINT32 *fb = (UINT32 *)params.gopMode.FrameBufferBase;
  UINT32 xres = params.gopMode.Info->PixelsPerScanLine;
  UINT32 yres = params.gopMode.Info->VerticalResolution;

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

  while(1); // Infinite loop, do not return back to UEFI
}