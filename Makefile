.POSIX:
.PHONY: all clean

SOURCES = bootloader/bootloader.c
OBJS = $(SOURCES:.c=.o)
DEPENDS = $(OBJS:.o=.d)
TARGET = BOOTX64.EFI
QEMU = ./qemu.sh
TESTFILE = test.txt

# Flat binary PIE kernel binary
KERNEL_BIN = kernel.sex
# ELF64 PIE kernel binar
KERNEL_ELF = kernel.elf
# PE32+ PIE kernel binary
KERNEL_PE = kernel.pe

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

all: $(TARGET) $(TESTFILE)
	cd $(DISK_IMG_FOLDER); \
	./$(DISK_IMG_PGM) -ad $(TESTFILE); \
	./$(QEMU)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $<
	cp $(TARGET) $(DISK_IMG_FOLDER)

$(TESTFILE):
	echo TESTING > $@
	cp $@ $(DISK_IMG_FOLDER)

# TODO
#$(KERNEL_BIN):

#$(KERNEL_ELF):

#$(KERNEL_PE):

-include $(DEPENDS)

clean:
	rm -rf $(TARGET) *.efi *.o *.d
	rm bootloader/*.o
