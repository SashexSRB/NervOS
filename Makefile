.POSIX:
.PHONY: all clean

SOURCES = bootloader/bootloader.c
OBJS = $(SOURCES:.c=.o)
DEPENDS = $(OBJS:.o=.d)
TARGET = BOOTX64.EFI
QEMU = ./qemu.sh
TESTFILE = test.txt

# Flat binary PIE kernel binary
#KERNEL = kernel.nx
# ELF64 PIE kernel binar
#KERNEL = kernel.elf
# PE32+ PIE kernel binary
KERNEL = kernel.pe

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

all: $(TARGET) $(KERNEL)
	cd $(DISK_IMG_FOLDER); \
	./$(DISK_IMG_PGM) -ad $(KERNEL); \
	./$(QEMU)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $<
	cp $(TARGET) $(DISK_IMG_FOLDER)

$(TESTFILE):
	echo TESTING > $@
	cp $@ $(DISK_IMG_FOLDER)

# Flat binary ELF/clang
# kernel.nx: kernel/kernel.c
# 	clang -c $(CFLAGS) -fPIE -o kernel.o $<
# 	ld -nostdlib -e kmain --oformat binary -pie -o $@ kernel.o
# 	cp $@ $(DISK_IMG_FOLDER)

# Flat binary MINGW GCC (PE)
# kernel.nx: kernel/kernel.c
# 	x86_64-w64-mingw32-gcc -c $(CFLAGS) -fPIE -o kernel.o $<
# 	x86_64-w64-mingw32-ld -nostdlib -e kmain -pie -o kernel.obj kernel.o
# 	objcopy -O binary kernel.obj $@
# 	cp $@ $(DISK_IMG_FOLDER)

# kernel.elf: kernel/kernel.c
# 	clang $(CFLAGS) -fPIE -e kmain -nostdlib -o $@ $<
# 	cp $@ $(DISK_IMG_FOLDER)

kernel.pe: kernel/kernel.c
	x86_64-w64-mingw32-gcc $(CFLAGS) -fPIE -e kmain -nostdlib -o $@ $<
	cp $@ $(DISK_IMG_FOLDER)

-include $(DEPENDS)

clean:
	rm -rf $(TARGET) *.efi *.o *.d *.nx *.elf *.pe
	rm -rf ugic/*.elf
	rm -rf ugic/*.nx
	rm -rf ugic/*.pe
	rm -rf ugic/*.efi 
	rm -rf ugic/*.EFI
	rm -rf ugic/*.hdd
	rm -rf ugic/*.TXT
	rm bootloader/*.o
