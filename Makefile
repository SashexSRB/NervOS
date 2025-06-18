.POSIX:
.PHONY: all clean

SOURCE = bootloader/bootloader.c
TARGET = BOOTX64.EFI

ifeq ($(OS), Windows_NT)
QEMU = ./qemu.bat
DISK_FLAGS = --vhd
else
QEMU = ./qemu.sh
DISK_FLAGS =
endif

CC = x86_64-w64-mingw32-gcc -Wl,--subsystem,10 -e efi_main

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

all: $(DISK_IMG_FOLDER)/$(DISK_IMG_PGM) $(TARGET)
	cp $(TARGET) $(DISK_IMG_FOLDER)
	cd $(DISK_IMG_FOLDER) && ./$(DISK_IMG_PGM) $(DISK_FLAGS)
	cd $(DISK_IMG_FOLDER) && $(QEMU)

$(DISK_IMG_FOLDER)/$(DISK_IMG_PGM):
	cd $(DISK_IMG_FOLDER) && $(MAKE)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(TARGET)
