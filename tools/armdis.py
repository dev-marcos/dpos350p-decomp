#!/usr/bin/env python3
"""Quick Capstone disassembly of the app image: armdis.py ADDR LEN [--thumb] [--words]"""
import argparse, struct
from capstone import Cs, CS_ARCH_ARM, CS_MODE_ARM, CS_MODE_THUMB
BASE = 0x40000000
ap = argparse.ArgumentParser()
ap.add_argument("addr", type=lambda s: int(s, 0)); ap.add_argument("length", type=lambda s: int(s, 0))
ap.add_argument("--thumb", action="store_true"); ap.add_argument("--words", action="store_true")
ap.add_argument("--image", default="build/extract/app.bin")
a = ap.parse_args()
img = open(a.image, "rb").read()
off = a.addr - BASE
buf = img[off:off + a.length]
if a.words:
    for i in range(0, len(buf) - 3, 4):
        print(f"{a.addr+i:08x}: {struct.unpack_from('<I', buf, i)[0]:08x}")
else:
    md = Cs(CS_ARCH_ARM, CS_MODE_THUMB if a.thumb else CS_MODE_ARM)
    md.skipdata = True
    for ins in md.disasm(buf, a.addr):
        print(f"{ins.address:08x}: {ins.bytes.hex():<9} {ins.mnemonic} {ins.op_str}")
