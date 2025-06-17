#!/bin/bash
#qemu-system-x86_64 -drive if=pflash,format=raw,unit=0,file=/usr/share/OVMF/OVMF_CODE.fd,readonly=on \
#-drive if=pflash,format=raw,unit=1,file=/usr/share/OVMF/OVMF_VARS.fd \
#-cdrom build/boot.iso -m 256 -serial stdio

qemu-system-x86_64 \
  -pflash /usr/share/ovmf/OVMF.fd \
  -cdrom build/boot.iso \
  -m 256 \
  -serial stdio