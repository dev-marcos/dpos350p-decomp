#!/usr/bin/env python3
"""Print the PC-relative literals loaded by a function: lits.py ADDR [--thumb]."""
import json, struct, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parent.parent
a = int(sys.argv[1], 0)
m = json.loads((ROOT / "build/analysis/map.json").read_text())
img = (ROOT / "build/extract/app.bin").read_bytes()
f = next(f for f in m["functions"] if f["start"] == a)
for la, sz in f["literals"]:
    v = struct.unpack_from("<I", img, la - 0x40000000)[0] if sz >= 4 else None
    print(f"{la:#010x}: {v:#010x}" if v is not None else f"{la:#010x}: (size {sz})")
