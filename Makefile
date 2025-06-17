# Toolchain
CC = x86_64-elf-gcc
LD_BOOTLOADER = ld
LD_KERNEL = ld
OBJCOPY = objcopy # Defined but not used for EFI conversion now
ELF2EFI = elf2efi

BUILD_DIR = build
ISO_DIR = iso
EFI_DIR = $(ISO_DIR)/EFI/BOOT

# Include and library paths for gnu-efi
EFI_INC = -I/usr/include/efi -I/usr/include/efi/x86_64
EFI_LIB = -L/usr/lib -lefi -lgnuefi

# Add --build-id=none to prevent linker from adding build IDs that objcopy might strip relocations due to.
# The -pie flag is crucial for creating a Position Independent Executable (PIE)
LD_BOOTLOADER_FLAGS = -T bootloader/efi.ld -nostdlib --entry efi_main --build-id=none -pie

all: $(BUILD_DIR)/boot.iso

# UEFI bootloader
$(BUILD_DIR)/BOOTX64.efi: bootloader/minimal.c
	mkdir -p $(BUILD_DIR)
# Compile C source to an ELF object file (with PIC, using -fpic)
	$(CC) $(EFI_INC) -fshort-wchar -ffreestanding -fpic -mno-red-zone -fno-stack-protector -fno-stack-check -Wall -c $< -o $(BUILD_DIR)/bootloader.o
# Link the ELF object into an ELF executable as a Position Independent Executable (PIE).
# The -pie flag forces the linker to generate R_X86_64_RELATIVE relocations,
# which elf2efi needs to create the PE/COFF base relocation table (.reloc).
	$(LD_BOOTLOADER) $(LD_BOOTLOADER_FLAGS) $(BUILD_DIR)/bootloader.o $(EFI_LIB) -o $(BUILD_DIR)/bootloader.elf

# --- Debugging Checks (Optional, can be removed once it works) ---
# Check if the intermediate ELF file is a PIE (should output "shared object")
	@echo "--- Verifying intermediate build/bootloader.elf ---"
	@file $(BUILD_DIR)/bootloader.elf
	@objdump -r $(BUILD_DIR)/bootloader.elf | grep R_X86_64_RELATIVE || echo "WARNING: No R_X86_64_RELATIVE relocations found in bootloader.elf (might be an issue)"
	@echo "---------------------------------------------------"
# --- End Debugging Checks ---

# Convert the PIE ELF executable to PE/COFF format (.efi) using elf2efi.
# elf2efi preserves the relocations properly and generates the .reloc section.
	$(ELF2EFI) $(BUILD_DIR)/bootloader.elf $(BUILD_DIR)/BOOTX64.efi

# --- Debugging Checks (Optional, can be removed once it works) ---
# Verify the final EFI file contains a .reloc section (crucial for relocatability)
	@echo "--- Verifying final build/BOOTX64.efi ---"
	@objdump -x $(BUILD_DIR)/BOOTX64.efi | grep ".reloc" > /dev/null && echo "INFO: build/BOOTX64.efi contains a .reloc section." || echo "WARNING: build/BOOTX64.efi DOES NOT contain a .reloc section. This is likely why it's not booting."
	@echo "-----------------------------------------"
# --- End Debugging Checks ---

# Kernel ELF
$(BUILD_DIR)/kernel.elf: kernel/kernel.c
	$(CC) -I kernel/inc -ffreestanding -O2 -nostdlib -c $< -o $(BUILD_DIR)/kernel.o
	$(LD_KERNEL) -T kernel/linker.ld -o $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.o

# Bootable ISO
$(BUILD_DIR)/boot.iso: $(BUILD_DIR)/BOOTX64.efi $(BUILD_DIR)/kernel.elf
	mkdir -p $(EFI_DIR)
	cp $(BUILD_DIR)/BOOTX64.efi $(EFI_DIR)/BOOTX64.efi
	cp ~/uefi-shell/Shell.efi $(EFI_DIR)/Shell.efi
	cp $(BUILD_DIR)/kernel.elf $(ISO_DIR)/kernel.elf
	rm -f $(BUILD_DIR)/fat.img
	dd if=/dev/zero of=$(BUILD_DIR)/fat.img bs=1M count=33
	mformat -i build/fat.img -F -v UEFI_BOOT ::
	mmd -i $(BUILD_DIR)/fat.img ::EFI
	mmd -i $(BUILD_DIR)/fat.img ::EFI/BOOT
	mcopy -i $(BUILD_DIR)/fat.img $(EFI_DIR)/BOOTX64.efi ::EFI/BOOT
	mcopy -i $(BUILD_DIR)/fat.img $(EFI_DIR)/Shell.efi ::EFI/BOOT
	mcopy -i $(BUILD_DIR)/fat.img $(ISO_DIR)/kernel.elf ::
	xorriso -as mkisofs \
	    --efi-boot fat.img \
	    --no-emul-boot \
	    --efi-boot-part --efi-boot-image --protective-msdos-label \
	    -o $(BUILD_DIR)/boot.iso \
	    $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)/EFI $(ISO_DIR)/kernel.elf

.PHONY: all clean
