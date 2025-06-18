CC = x86_64-w64-mingw32-gcc

BUILD_DIR = build
ISO_DIR = iso
EFI_DIR = $(ISO_DIR)/EFI/BOOT

CC_FLAGS = -std=c17 -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -nostdlib -Wl,--subsystem,10 -e efi_main -o $(BUILD_DIR)/BOOTX64.EFI

all: $(BUILD_DIR)/boot.iso

# Bootloader
$(BUILD_DIR)/BOOTX64.EFI: bootloader/bootloader.c
	@echo "-------- LOG: Creating Build Directory... -------"
	mkdir -p $(BUILD_DIR)
	@echo "--------------- LOG: Compiling... ---------------"
	$(CC) bootloader/bootloader.c $(CC_FLAGS)
	@echo "--- LOG: Verifying final build/BOOTX64.EFI... ---"
	@objdump -x $(BUILD_DIR)/BOOTX64.EFI > BL_DUMP.txt
	@echo "-------------------------------------------------"


# Bootable ISO
$(BUILD_DIR)/boot.iso: $(BUILD_DIR)/BOOTX64.EFI
	mkdir -p $(EFI_DIR)
	cp $(BUILD_DIR)/BOOTX64.EFI $(EFI_DIR)/BOOTX64.EFI
	rm -f $(BUILD_DIR)/fat.img
	dd if=/dev/zero of=$(BUILD_DIR)/fat.img bs=1M count=33
	mformat -i build/fat.img -F -v UEFI_BOOT ::
	mmd -i $(BUILD_DIR)/fat.img ::EFI
	mmd -i $(BUILD_DIR)/fat.img ::EFI/BOOT
	mcopy -i $(BUILD_DIR)/fat.img $(EFI_DIR)/BOOTX64.EFI ::EFI/BOOT
	xorriso -as mkisofs \
	    --efi-boot fat.img \
	    --no-emul-boot \
	    --efi-boot-part --efi-boot-image --protective-msdos-label \
	    -o $(BUILD_DIR)/boot.iso \
	    $(BUILD_DIR)

clean: 
	rm -rf $(BUILD_DIR) $(ISO_DIR)/EFI

.PHONY: all clean