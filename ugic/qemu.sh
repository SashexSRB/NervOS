#!/bin/sh

# Unbind the USB mouse from host (optional but helps avoid conflicts)

# Launch QEMU with USB passthrough
qemu-system-x86_64 \
-drive format=raw,file=test.hdd \
-bios bios64.bin \
-m 256M \
-vga std \
-display gtk,gl=on,zoom-to-fit=off,window-close=on \
-name nOS \
-machine q35 \
-usb \
-device usb-mouse \
-rtc base=localtime \
-net none \
-serial stdio

#-bios /usr/share/OVMF/x64/OVMF.4m.fd \