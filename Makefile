# Toolchain
CC = x86_64-elf-gcc
LD_BOOTLOADER = ld
LD_KERNEL = x86_64-elf-ld

BUILD_DIR = build
ISO_DIR = iso
EFI_DIR = $(ISO_DIR)/EFI/BOOT

# Include and library paths for gnu-efi
EFI_INC = -I/usr/include/efi -I/usr/include/efi/x86_64
EFI_LIB = -L/usr/lib -lefi -lgnuefi

all: $(BUILD_DIR)/boot.iso

# UEFI bootloader
$(BUILD_DIR)/BOOTX64.EFI: bootloader/bootloader.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(EFI_INC) -ffreestanding -fpic -mno-red-zone -fno-stack-protector -Wall -c $< -o $(BUILD_DIR)/bootloader.o
	$(LD_BOOTLOADER) -T bootloader/efi.ld -nostdlib -shared -Bsymbolic \
		-e efi_main $(BUILD_DIR)/bootloader.o $(EFI_LIB) -o $(BUILD_DIR)/bootloader.elf
	objcopy --subsystem efi-app -O pei-x86-64 $(BUILD_DIR)/bootloader.elf $(BUILD_DIR)/BOOTX64.EFI

# Kernel ELF
$(BUILD_DIR)/kernel.elf: kernel/kernel.c
	$(CC) -I kernel/inc -ffreestanding -O2 -nostdlib -c $< -o $(BUILD_DIR)/kernel.o
	$(LD_KERNEL) -T kernel/linker.ld -o $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.o

# Bootable ISO
$(BUILD_DIR)/boot.iso: $(BUILD_DIR)/BOOTX64.EFI $(BUILD_DIR)/kernel.elf
	mkdir -p $(EFI_DIR)
	cp $(BUILD_DIR)/BOOTX64.EFI $(EFI_DIR)/BOOTX64.EFI
	cp $(BUILD_DIR)/kernel.elf $(ISO_DIR)/kernel.elf
	rm -f $(BUILD_DIR)/fat.img
	dd if=/dev/zero of=$(BUILD_DIR)/fat.img bs=512 count=93750
	mkfs.fat -F 32 -n "UEFI_BOOT" $(BUILD_DIR)/fat.img
	mmd -i $(BUILD_DIR)/fat.img ::EFI
	mmd -i $(BUILD_DIR)/fat.img ::EFI/BOOT
	mcopy -i $(BUILD_DIR)/fat.img $(EFI_DIR)/BOOTX64.EFI ::EFI/BOOT
	mcopy -i $(BUILD_DIR)/fat.img $(ISO_DIR)/kernel.elf ::
	cp $(BUILD_DIR)/fat.img $(ISO_DIR)/fat.img
	# Create ISO with the FAT image
	xorriso -as mkisofs -R -f --efi-boot fat.img -no-emul-boot \
		-o $(BUILD_DIR)/boot.iso $(ISO_DIR)
	rm $(ISO_DIR)/fat.img

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)/EFI $(ISO_DIR)/kernel.elf