#!/usr/bin/env python3
"""Minimal Android boot.img v2 writer (no dtb). usage: kernel ramdisk out [cmdline]"""
import struct, sys, hashlib
k, r, o = sys.argv[1:4]; cmd = sys.argv[4] if len(sys.argv) > 4 else "console=null"
K, R = open(k, 'rb').read(), open(r, 'rb').read()
P = 2048; pad = lambda b: b + b'\0' * (-len(b) % P)
h = b'ANDROID!' + struct.pack('<10I', len(K), 0x40080000, len(R), 0x47c80000, 0, 0x40000000 + 0x0, 0x4bc80000, P, 2, 0x16000182)
h += b'\0' * 16 + cmd.encode().ljust(512, b'\0')
h += hashlib.sha1(K + R).digest().ljust(32, b'\0') + b'\0' * 1024
h += struct.pack('<IQI', 0, 0, 1660) + struct.pack('<IQ', 0, 0)
open(o, 'wb').write(pad(h.ljust(P, b'\0')) + pad(K) + pad(R))
