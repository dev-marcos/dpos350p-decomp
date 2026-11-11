#!/usr/bin/env python3
"""Compare a function compiled into an object file against the original image.

  cmpfunc.py OBJ SYMBOL ADDR [--thumb] [--len N] [--show]

Prints: exact byte match, sizes, and an instruction-level similarity (difflib ratio over
normalised instructions: branch targets, PC-relative offsets and literal values are
abstracted, so layout differences do not count as mismatches).
"""
import argparse
import difflib
import re
import subprocess
import sys
from pathlib import Path

from capstone import Cs, CS_ARCH_ARM, CS_MODE_ARM, CS_MODE_THUMB

ROOT = Path(__file__).resolve().parent.parent
BASE = 0x40000000


def obj_func(obj, sym):
    out = subprocess.run(["arm-none-eabi-nm", "-S", "--defined-only", obj], capture_output=True, text=True,
                         check=True).stdout
    for line in out.splitlines():
        p = line.split()
        if len(p) == 4 and p[3] == sym:
            addr, size = int(p[0], 16), int(p[1], 16)
            break
    else:
        return None, None
    sec = f".text.{sym}"
    tmp = ROOT / "build/probe/_sec.bin"
    tmp.parent.mkdir(parents=True, exist_ok=True)
    r = subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", "-j", sec, obj, str(tmp)], capture_output=True)
    if r.returncode != 0 or not tmp.exists() or tmp.stat().st_size == 0:
        subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text", obj, str(tmp)], check=True)
        data = tmp.read_bytes()[addr & ~1:(addr & ~1) + size]
    else:
        data = tmp.read_bytes()[:size]
    return data, bool(addr & 1)


def norm(code, addr, thumb):
    md = Cs(CS_ARCH_ARM, CS_MODE_THUMB if thumb else CS_MODE_ARM)
    md.skipdata = True
    out = []
    for i in md.disasm(code, addr):
        op = i.op_str
        if i.mnemonic.startswith(("b", "cb")) and "#0x" in op:
            op = re.sub(r"#0x[0-9a-f]+", "<T>", op)
        op = re.sub(r"\[pc, #-?0x[0-9a-f]+\]", "[pc,<L>]", op)
        out.append(f"{i.mnemonic} {op}")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("obj"); ap.add_argument("sym"); ap.add_argument("addr", type=lambda s: int(s, 0))
    ap.add_argument("--len", type=lambda s: int(s, 0)); ap.add_argument("--thumb", action="store_true")
    ap.add_argument("--show", action="store_true")
    a = ap.parse_args()
    img = (ROOT / "build/extract/app.bin").read_bytes()
    mine, th = obj_func(a.obj, a.sym)
    if mine is None:
        print(f"{a.sym}: not in {a.obj} (inlined or removed)")
        return 2
    n = a.len or len(mine)
    orig = img[a.addr - BASE:a.addr - BASE + n]
    thumb = a.thumb or th
    A, B = norm(orig, a.addr, thumb), norm(mine, 0, thumb)
    ratio = difflib.SequenceMatcher(None, A, B, autojunk=False).ratio()
    exact = mine == orig
    print(f"{a.sym}: orig {n} B / {len(A)} insns, mine {len(mine)} B / {len(B)} insns, "
          f"similarity {ratio:.3f}, exact={'YES' if exact else 'no'}")
    if a.show:
        for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, A, B, autojunk=False).get_opcodes():
            for k in range(max(i2 - i1, j2 - j1)):
                l = A[i1 + k] if i1 + k < i2 else ""
                r = B[j1 + k] if j1 + k < j2 else ""
                print(f"{'  ' if tag == 'equal' else '!='} {l:<40} | {r}")
    return 0 if exact else 1


if __name__ == "__main__":
    sys.exit(main())
