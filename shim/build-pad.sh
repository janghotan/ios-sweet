#!/bin/bash
# Milestone 0 image for realme Pad. Needs aarch64-linux-gnu-gcc and mkbootimg.
set -euo pipefail
cd "$(dirname "$0")"
CC=aarch64-linux-gnu-gcc
F="-O2 -ffreestanding -nostdlib -fno-pic -mgeneral-regs-only -mstrict-align -fno-stack-protector -DPAD -Wall"
mkdir -p out-pad
$CC $F -c start.S -o out-pad/start.o
$CC $F -c pad_hello.c -o out-pad/hello.o
$CC $F -c console.c -o out-pad/console.o
$CC $F -static -T shim.ld -Wl,--build-id=none -Wl,--no-warn-rwx-segments -Wl,-N out-pad/*.o -o out-pad/shim.elf
aarch64-linux-gnu-objcopy -O binary out-pad/shim.elf out-pad/Image
size=$(aarch64-linux-gnu-nm out-pad/shim.elf | awk '$3=="_end"{print strtonum("0x"$1)}')
truncate -s "$size" out-pad/Image
(cd out-pad && rm -rf rd && mkdir rd && cd rd && echo x > x && find . | cpio -o -H newc --quiet | gzip -9 > ../ramdisk.bin)
[ -f ../dtb/pad.dtb ] || { echo "missing dtb/pad.dtb: run python3 tools/extract_dtb.py <your boot.img> dtb/pad.dtb" >&2; exit 1; }
python3 ../tools/mkboot_v2.py out-pad/Image out-pad/ramdisk.bin out-pad/pad-boot.img console=null ../dtb/pad.dtb
ls -l out-pad/pad-boot.img

[ -f ~/recovery_stock.img ] && python3 ../tools/mkboot_like.py ~/recovery_stock.img out-pad/Image out-pad/ramdisk.bin out-pad/pad-recovery.img && ls -l out-pad/pad-recovery.img
