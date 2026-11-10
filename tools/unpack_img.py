#!/usr/bin/env python3
"""unpack_img.py stock.img kernel_out ramdisk_out"""
import struct, sys
d = open(sys.argv[1], 'rb').read()
ks, _, rs, _, ss, _, _, P = struct.unpack('<8I', d[8:40])
up = lambda n: (n + P - 1) // P * P
open(sys.argv[2], 'wb').write(d[P:P + ks])
open(sys.argv[3], 'wb').write(d[P + up(ks):P + up(ks) + rs])
print('kernel', ks, 'ramdisk', rs, 'kernel head', d[P:P+8].hex(), 'magic@0x38', d[P+0x38:P+0x3c])
