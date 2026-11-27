#!/usr/bin/env python3
"""Relocation test: link the cluster sections in another order and boot the result.

  relink_test.py            (after `make check`) -> build/relink/app.bin, build/relink/reloc.json
  relink_test.py --shuffle N   random order (seed N) instead of the reversed one

Every cluster (asm/sections/sec_*.s) is placed somewhere else: the vector page stays first,
then the rodata/RW blob, then all the other clusters in reverse order. The image keeps its
size, so it still ends where the ZI region starts (0x400954A4, fixed by the scatter-load
table). If any reference inside the image were still a number instead of a symbol, the moved
code would read or jump to the wrong place.

The emulator hooks some routines and starts at RESET by stock address; reloc.json lists
[stock start, stock end, delta] per cluster, and `node tools/sim/tests/boot.mjs --reloc` moves
those addresses with their code. Then compare its screen with the stock image's.
"""
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/relink"
CROSS = "arm-none-eabi-"
ZI_BASE = 0x400954A4


def stock_layout():
    """[(section, start, size)] from the matching link (build/app.map)."""
    lines = (ROOT / "build/app.map").read_text().splitlines()
    out = []
    for i, l in enumerate(lines):
        m = re.match(r"^ (\.text\.[0-9a-f]{8})\s*$", l)
        if m and i + 1 < len(lines):
            n = re.match(r"^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s", lines[i + 1])
            if n:
                out.append((m.group(1), int(n.group(1), 16), int(n.group(2), 16)))
    return out


def main():
    lay = stock_layout()
    assert lay and lay[0][0] == ".text.40000000", "run `make check` first"
    by = {s: (a, n) for s, a, n in lay}
    rodata = ".text.4008cfb8"            # rodata + RW blob
    garbage = ".text.400954a4"           # bytes past the ZI base: zeroed at boot, left out
    middle = [s for s, _, _ in lay if s not in (".text.40000000", rodata, garbage)]
    if "--shuffle" in sys.argv:
        import random
        # sections whose size is not a multiple of 4 (Thumb code) stay as one block in their
        # original order, so no alignment padding appears and the image keeps its size
        odd = [k for k, s in enumerate(middle) if by[s][1] % 4]
        items = [[s] for s in middle]
        if odd:
            items = items[:odd[0]] + [middle[odd[0]:odd[-1] + 1]] + items[odd[-1] + 1:]
        random.Random(int(sys.argv[sys.argv.index("--shuffle") + 1])).shuffle(items)
        middle = [s for it in items for s in it]
        order = [".text.40000000"] + middle
        order.insert(len(order) // 2, rodata)
    else:
        order = [".text.40000000", rodata] + middle[::-1]
    OUT.mkdir(parents=True, exist_ok=True)
    keep = "\n".join(f"        KEEP(*({s}))" for s in order)
    (OUT / "relink.ld").write_text(f"""OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)
SECTIONS
{{
    .text 0x40000000 : {{
{keep}
    }}
    __image_end = .;
    ASSERT(__image_end <= {ZI_BASE:#x}, "relinked image runs into the ZI region")
    /DISCARD/ : {{ *(.text.400954a4) *(.ARM.attributes) *(.comment) *(.note*) *(.ARM.exidx*) }}
}}
""")
    objs = (ROOT / "build/objs.rsp").read_text().split()
    subprocess.run([CROSS + "ld", "-T", str(OUT / "relink.ld"), "-Map=" + str(OUT / "app.map"),
                    "-o", str(OUT / "app.elf"), *objs], check=True, cwd=ROOT)
    subprocess.run([CROSS + "objcopy", "-O", "binary", str(OUT / "app.elf"), str(OUT / "app.bin")],
                   check=True)
    # where each section landed
    new = {}
    lines = (OUT / "app.map").read_text().splitlines()
    for i, l in enumerate(lines):
        m = re.match(r"^ (\.text\.[0-9a-f]{8})\s*$", l)
        if m and i + 1 < len(lines):
            n = re.match(r"^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s", lines[i + 1])
            if n:
                new[m.group(1)] = int(n.group(1), 16)
    spans = [[a, a + n, new[s] - a] for s, (a, n) in by.items() if s in new]
    moved = sum(1 for _, _, d in spans if d)
    (OUT / "reloc.json").write_text(json.dumps(spans))
    size = (OUT / "app.bin").stat().st_size
    print(f"{len(spans)} sections, {moved} moved; image {size} bytes (stock 0x957E0 with the "
          f"0x{0x400957E0 - ZI_BASE:X} bytes past the ZI base left out)")
    if moved < len(spans) - 1:
        sys.exit("not every section moved")


if __name__ == "__main__":
    main()
