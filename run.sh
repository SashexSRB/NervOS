qemu-system-x86_64 \
  -pflash /usr/share/ovmf/OVMF.fd \
  -cdrom build/boot.iso \
  -m 256 \
  -vga std \
  -name NervOS \
  -machine q35 \
  -serial stdio \
  -net none