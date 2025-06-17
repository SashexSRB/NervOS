#!/bin/bash
qemu-system-x86_64 -drive if=pflash,format=raw,unit=0,file=OVMF_CODE.fd,readonly=on \
-drive if=pflash,format=raw,unit=1,file=OVMF_VARS.fd \
-cdrom build/boot.iso -m 256 -serial stdio