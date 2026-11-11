#!/usr/bin/env python3
"""Phase 4 probe: compile FatFs R0.13b with GCC flag variants and compare functions.

Usage (inside dpos-re):  python3 tools/compiler_probe/probe_gcc.py [--cc arm-none-eabi-gcc]
Writes build/probe/gcc_<tag>/ff.o and prints a similarity table (see tools/cmpfunc.py).
"""
import argparse
import itertools
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import cmpfunc  # noqa: E402
import difflib  # noqa: E402

FATFS = ROOT / "third_party/fatfs/R0.13b/source"
CONF = ROOT / "tools/compiler_probe/fatfs"

# original addresses (Phase 2 map; identification evidence in docs/COMPILER.md)
TARGETS = {
    "ld_word": 0x40056A70,
    "mem_cmp": 0x40056BBC,
    "move_window": 0x40056D80,
    "get_ldnumber": 0x400593DC,
    "check_fs": 0x40059454,
    "f_mkfs": 0x4005B1F8,
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cc", default="arm-none-eabi-gcc")
    a = ap.parse_args()
    m = json.loads((ROOT / "build/analysis/map.json").read_text())
    size = {f["start"]: f["end"] - f["start"] for f in m["functions"]}
    img = (ROOT / "build/extract/app.bin").read_bytes()
    ver = subprocess.run([a.cc, "-dumpfullversion"], capture_output=True, text=True).stdout.strip()
    opts = ["-O0", "-O1", "-O2", "-Os", "-O3"]
    floats = ["-mfloat-abi=hard", "-mfloat-abi=soft"]
    inl = ["", "-fno-inline"]
    # ff.c includes "ffconf.h" from its own directory first: build from a merged copy
    src = ROOT / "build/probe/fatfs_src"
    src.mkdir(parents=True, exist_ok=True)
    for f in ("ff.c", "ff.h", "diskio.h", "integer.h"):
        if (FATFS / f).exists():
            (src / f).write_bytes((FATFS / f).read_bytes())
    (src / "ffconf.h").write_bytes((CONF / "ffconf.h").read_bytes())
    print(f"compiler {a.cc} {ver}")
    print(f"{'flags':<44}" + "".join(f"{k:>13}" for k in TARGETS))
    best = {}
    for o, fl, ni in itertools.product(opts, floats, inl):
        tag = f"{ver}{o}{fl}{ni}".replace("=", "_").replace("-", "")
        out = ROOT / f"build/probe/gcc_{tag}"
        out.mkdir(parents=True, exist_ok=True)
        obj = out / "ff.o"
        cmd = [a.cc, "-c", "-marm", "-mcpu=cortex-a7", "-mfpu=neon-vfpv4", fl, o, "-ffunction-sections",
               "-fno-common", "-I", str(src), str(src / "ff.c"), "-o", str(obj)]
        if ni:
            cmd.insert(2, ni)
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode:
            print(f"{o} {fl} {ni}: compile failed: {r.stderr[:200]}")
            continue
        row = []
        for sym, addr in TARGETS.items():
            mine, th = cmpfunc.obj_func(str(obj), sym)
            if mine is None:
                row.append("inlined")
                continue
            orig = img[addr - 0x40000000: addr - 0x40000000 + size[addr]]
            A, B = cmpfunc.norm(orig, addr, False), cmpfunc.norm(mine, 0, th)
            r_ = difflib.SequenceMatcher(None, A, B, autojunk=False).ratio()
            exact = mine == orig
            row.append(("EXACT" if exact else f"{r_:.2f}") + f"/{len(mine)}")
            best[sym] = max(best.get(sym, (0, "")), (r_, f"{o} {fl} {ni}"))
        print(f"{(o + ' ' + fl + ' ' + ni):<44}" + "".join(f"{c:>13}" for c in row))
    print("original sizes:", {k: size[v] for k, v in TARGETS.items()})
    print("best per function:", {k: f"{v[0]:.2f} ({v[1].strip()})" for k, v in best.items()})


if __name__ == "__main__":
    main()
