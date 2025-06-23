.POSIX:
.PHONY: all clean

SOURCES = bootloader/bootloader.c
OBJS = $(SOURCES:.c=.o)
DEPENDS = $(OBJS:.o=.d)
TARGET = BOOTX64.EFI
QEMU = ./qemu.sh

CC = x86_64-w64-mingw32-gcc -Wl,--subsystem,10 -e efi_main

LDFLAGS = \
	-nostdlib \
	-Wl,--subsystem,10 \
	-e efi_main

CFLAGS = \
	-std=c17 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-mno-red-zone \
	-ffreestanding \
	-nostdlib

DISK_IMG_FOLDER = ugic
DISK_IMG_PGM = writeGpt

all: $(TARGET)
	cd $(DISK_IMG_FOLDER); \
	./$(DISK_IMG_PGM); \
	./$(QEMU)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $<
	cp $(TARGET) $(DISK_IMG_FOLDER)

-include $(DEPENDS)

clean:
	rm -rf $(TARGET) *.efi *.o *.d
