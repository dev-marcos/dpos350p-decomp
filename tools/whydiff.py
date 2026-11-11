#!/usr/bin/env python3
"""Locate the first divergence between the linked image and the original.

Links build/asm/*.o without the size ASSERT, then reports:
  * the first unit whose section address differs from its original address
  * the first differing byte and the instruction/line it belongs to
Usage (inside dpos-re, after `make` failed):  python3 tools/whydiff.py
"""
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BASE = 0x40000000
B = ROOT / "build"

script = (ROOT / "ld/dpos350p.ld").read_text()
script = re.sub(r"^\s*ASSERT\(.*$", "", script, flags=re.M)
(B / "nocheck.ld").write_text(script)
subprocess.run(["arm-none-eabi-ld", "-T", str(B / "nocheck.ld"), "-Map", str(B / "nocheck.map"),
                "-o", str(B / "nocheck.elf"), "@" + str(B / "objs.rsp")], check=True, cwd=ROOT)
subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", str(B / "nocheck.elf"), str(B / "nocheck.bin")],
               check=True)

# section placement from the map: " .text.40019af4  0x40019af4  0x98 build/asm/func_40019af4.o"
mp = (B / "nocheck.map").read_text()
for m in re.finditer(r"^\s*\.text\.([0-9a-f]{8})\s*\n?\s*0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+)", mp, re.M):
    want, got, size = int(m.group(1), 16), int(m.group(2), 16), int(m.group(3), 16)
    if want != got:
        print(f"first misplaced unit: {m.group(4)} expected 0x{want:08x} got 0x{got:08x}")
        break

orig = (B / "extract/app.bin").read_bytes()
new = (B / "nocheck.bin").read_bytes()
print(f"sizes: original {len(orig)} rebuilt {len(new)}")
for i in range(min(len(orig), len(new))):
    if orig[i] != new[i]:
        a = BASE + i
        print(f"first differing byte at 0x{a:08x}: orig {orig[i:i+8].hex()} new {new[i:i+8].hex()}")
        # find the unit
        units = sorted(int(x.group(1), 16) for x in re.finditer(r"\.text\.([0-9a-f]{8})", mp))
        u = max(x for x in units if x <= a)
        print(f"unit 0x{u:08x} (asm/func_{u:08x}.s or data/vectors)")
        break
else:
    print("no differing byte in the common prefix")
