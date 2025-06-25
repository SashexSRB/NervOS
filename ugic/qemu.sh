#!/bin/sh

# Unbind the USB mouse from host (optional but helps avoid conflicts)

# Launch QEMU with USB passthrough
sudo qemu-system-x86_64 \
-drive format=raw,file=test.hdd \
-bios /usr/share/ovmf/OVMF.fd \
-m 256M \
-vga std \
-display gtk,gl=on,zoom-to-fit=off,window-close=on \
-name nOS \
-machine q35 \
-usb \
-device qemu-xhci,id=xhci \
-device usb-tablet \
-rtc base=localtime \
-net none