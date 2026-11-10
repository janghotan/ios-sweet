#!/usr/bin/env python3
"""extract_dtb.py boot.img out.dtb : pull the v2 dtb section out of a stock boot.img."""
import struct, sys
d = open(sys.argv[1], 'rb').read()
assert d[:8] == b'ANDROID!' and struct.unpack('<I', d[40:44])[0] == 2
ks, _, rs, _, ss, _, _, P = struct.unpack('<8I', d[8:40])
rdtbo = struct.unpack('<I', d[1632:1636])[0]
dsz = struct.unpack('<I', d[1648:1652])[0]
up = lambda n: (n + P - 1) // P * P
off = P + up(ks) + up(rs) + up(ss) + up(rdtbo)
open(sys.argv[2], 'wb').write(d[off:off + dsz])
print('dtb', dsz, 'bytes at', off, 'magic', d[off:off+4].hex())
