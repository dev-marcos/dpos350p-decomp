#!/usr/bin/env python3
"""Write nonmatching/platform/func_*.c from a compact spec, and the matching symbols/manual.csv rows.

The C bodies are transcriptions of the stock disassembly; names and structure follow EasyUSB
(see docs/PLATFORM.md). Usage: python3 tools/gen_platform.py [--csv]   (--csv prints manual.csv rows)
"""
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "nonmatching/platform"

HDR = """/*
 * {name} -- stock 0x{addr:08X} ({size} bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * {desc}
 */
#include "hw/platform.h"

"""

# (address, size, name, source, description, body)
SPEC = []


def f(addr, size, name, src, desc, body):
    SPEC.append((addr, size, name, src, desc, body.strip(chr(10)) + chr(10)))


for _part in sorted((ROOT / "tools/platform_spec").glob("*.py")):   # platform driver groups
    exec(compile(_part.read_text(), str(_part), "exec"))


def main():
    if "--csv" in sys.argv:
        for addr, size, name, src, desc, body in SPEC:
            origin = f"this-project:phase5 ({src})"
            print(f"0x{addr:08X},{name},function,DERIVED,{origin},")
        return
    OUT.mkdir(parents=True, exist_ok=True)
    changed = 0
    for addr, size, name, src, desc, body in SPEC:
        p = OUT / f"func_{addr:08x}.c"
        text = HDR.format(name=name, addr=addr, size=size, desc=desc) + body
        if p.exists() and p.read_text() == text:
            continue            # unchanged: rewriting every file invites transient Windows locks on /mnt/f
        for attempt in range(5):
            try:
                p.write_text(text)
                break
            except OSError:     # EINVAL from a short-lived lock (indexer/antivirus) on the Windows side
                if attempt == 4:
                    raise
                time.sleep(0.5)
        changed += 1
    print(f"{len(SPEC)} files in {OUT.relative_to(ROOT)} ({changed} written)")


if __name__ == "__main__":
    main()
