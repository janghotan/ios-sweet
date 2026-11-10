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
# MTK LK: base 0x40000000, kernel at 0x40080000
mkbootimg --header_version 2 --kernel out-pad/Image --ramdisk out-pad/ramdisk.bin \
  --base 0x40000000 --kernel_offset 0x80000 --ramdisk_offset 0x4000000 --second_offset 0 \
  --tags_offset 0x7c08000 --dtb /dev/null --dtb_offset 0x7c08000 --pagesize 2048 \
  --cmdline "console=null" -o out-pad/pad-boot.img
ls -l out-pad/pad-boot.img
