#!/usr/bin/env python3
"""mkboot_like.py stock.img kernel ramdisk out.img : stock header/dtb/addresses, our kernel+ramdisk."""
import struct, sys, hashlib
s, k, r, o = sys.argv[1:5]
d = open(s, 'rb').read(); assert d[:8] == b'ANDROID!'
ks, ka, rs, ra, ss, sa, ta, P, hv, osv = struct.unpack('<10I', d[8:48])
print('stock: hdr v%d page %d kaddr %#x raddr %#x tags %#x kernel %d ramdisk %d second %d' % (hv, P, ka, ra, ta, ks, rs, ss))
if hv > 2: sys.exit('header v%d unsupported' % hv)
up = lambda n: (n + P - 1) // P * P
pad = lambda b: b + b'\0' * (-len(b) % P)
off = P + up(ks) + up(rs) + up(ss)
rdtbo = d[off:off + struct.unpack('<I', d[1632:1636])[0]] if hv >= 1 else b''
off += up(len(rdtbo))
dsz = struct.unpack('<I', d[1648:1652])[0] if hv >= 2 else 0
dtb = d[off:off + dsz]
K, R = open(k, 'rb').read(), open(r, 'rb').read()
h = bytearray(d[:P])
h[8:40] = struct.pack('<8I', len(K), ka, len(R), ra, 0, sa, ta, P)
h[576:608] = hashlib.sha1(K + R).digest().ljust(32, b'\0')
if hv >= 1:
    h[1636:1644] = struct.pack('<Q', P + up(len(K)) + up(len(R)))
open(o, 'wb').write(bytes(h) + pad(K) + pad(R) + pad(rdtbo) + pad(dtb))
print('wrote', o)
