#!/usr/bin/env python3
"""Build every nonmatching/**/*.c (one stock function each, see tools/srcindex.py) as a standalone test image.

Each C file is compiled with GCC (only for behavioural checking, see docs/COMPILER.md §6)
and linked alone at SCRATCH (0x48000000). Every symbol it references but does not define
resolves to the ORIGINAL address from symbols/symbols_2025-06-19.csv, so a test exercises
exactly one rewritten function against the stock callees.

Output per function: build/nonmatching/func_XXXXXXXX.{o,elf,bin,json}
  json = {"orig": addr, "name": sym, "entry": addr_in_scratch, "thumb": bool, "key": cache key}

Cache, in two levels kept in the json: "okey" hashes the C file, every header under include/,
the compiler flags and this script; "key" adds the address each undefined symbol of the object
resolves to (listed in "undef"). A build is skipped when both match, so adding unrelated symbols
does not rebuild anything. The link script lists only the symbols the object uses. --force
rebuilds everything. The files are compiled in parallel (one job per CPU; -j N to change).
"""
import argparse
import csv
import hashlib
import json
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
try:
    from tools import srcindex                # noqa: E402
except ImportError:                           # run as a script from tools/
    import srcindex                           # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
SCRATCH = 0x48000000
CC = "arm-none-eabi-gcc"
CFLAGS = ["-c", "-marm", "-mcpu=cortex-a7", "-mfpu=vfpv4", "-mfloat-abi=hard", "-O2",
          "-ffreestanding", "-fno-builtin", "-fsigned-char", "-fno-tree-loop-distribute-patterns", "-ffp-contract=off", "-fno-common", "-ffunction-sections", "-fdata-sections",
          "-Wall", "-Wno-unused", "-I", str(ROOT / "include")]
OUT = ROOT / "build/nonmatching"


class Symbols(dict):
    """Name -> original address. Besides the listed names it resolves d_XXXXXXXX (include/image.h:
    a text, table or initialised global of the image, named after its address) to that address."""
    _D = re.compile(r"d_([0-9a-f]{8})$")

    def __contains__(self, k):
        return dict.__contains__(self, k) or bool(self._D.match(k))

    def __missing__(self, k):
        m = self._D.match(k)
        if not m:
            raise KeyError(k)
        return int(m.group(1), 16)

    def get(self, k, d=None):
        return self[k] if k in self else d


def original_symbols():
    syms = Symbols()
    for r in csv.DictReader(open(ROOT / "symbols/symbols_2025-06-19.csv")):
        if r["type"].startswith("function"):
            a = int(r["address"], 16)
            if r["type"] == "function_thumb":
                a |= 1
            syms[r["name"]] = a
    # curated names may not be in the generated CSV yet (if `make map` was not re-run)
    for r in csv.DictReader(open(ROOT / "symbols/manual.csv")):
        if r["type"] in ("function", "data"):
            syms.setdefault(r["name"], int(r["address"], 16))
    return syms


_BASE = {}


def _base_key():
    """hash of what changes the object file apart from the C file itself (per CFLAGS value):
    compiler, flags, every header, this script"""
    k = (CC, tuple(CFLAGS))
    if k not in _BASE:
        h = hashlib.sha256(repr(k).encode())
        for f in sorted((ROOT / "include").rglob("*.h")) + [Path(__file__)]:
            h.update(str(f.relative_to(ROOT)).encode() + b"\0" + f.read_bytes())
        _BASE[k] = h.hexdigest()
    return _BASE[k]


def object_key(src):
    return hashlib.sha256((_base_key() + "\0").encode() + src.read_bytes()).hexdigest()


def link_key(okey, undef, syms):
    """object key + the address every undefined symbol resolves to"""
    return hashlib.sha256((okey + "|" + ";".join(f"{n}={syms.get(n)}" for n in sorted(undef))).encode()).hexdigest()


def fix_thumb_calls(elf, binf, base, syms):
    """Make ARM calls into stock Thumb code switch state, as the stock BLX does.

    The stock symbols are absolute linker-script values, so ld does not know which ones are Thumb
    and links every call as BL: the callee would then run in ARM state. Using the relocations kept
    by `ld -q`, each R_ARM_CALL to a symbol with an odd address (Thumb) is rewritten as BLX imm.
    A B or BL<cond> (R_ARM_JUMP24) to Thumb code cannot switch state: such names are returned."""
    thumb = {n for n, a in syms.items() if a & 1}
    out = subprocess.run(["arm-none-eabi-readelf", "-rW", str(elf)], capture_output=True, text=True,
                         check=True).stdout
    code = bytearray(Path(binf).read_bytes())
    bad, fixed = set(), 0
    for l in out.splitlines():
        f = l.split()
        if len(f) < 5 or f[2] not in ("R_ARM_CALL", "R_ARM_JUMP24") or f[4] not in thumb:
            continue
        p, n = int(f[0], 16), f[4]
        o = p - base
        if not 0 <= o < len(code):
            continue
        w = int.from_bytes(code[o:o + 4], "little")
        if (w & 0x0E000000) != 0x0A000000 or (w >> 28) == 0xF:
            continue                                  # not B/BL (already BLX)
        imm = w & 0xFFFFFF
        d = (syms[n] & ~1) - (p + 8)
        if (imm - (0x1000000 if imm & 0x800000 else 0)) * 4 != d:
            continue                                  # goes through a veneer (out of range): it interworks
        if f[2] == "R_ARM_JUMP24" or (w & 0xFF000000) != 0xEB000000:
            bad.add(n)                                # B or BL<cond> straight into Thumb code
            continue
        code[o:o + 4] = (0xFA000000 | (((d >> 1) & 1) << 24) | ((d >> 2) & 0xFFFFFF)).to_bytes(4, "little")
        fixed += 1
    if fixed:
        Path(binf).write_bytes(bytes(code))
    return sorted(bad)


def build(src, syms, force=False):
    orig = srcindex.orig_of(src)
    stem = OUT / f"func_{orig:08x}"
    OUT.mkdir(parents=True, exist_ok=True)
    okey = object_key(src)
    js, binf = Path(str(stem) + ".json"), Path(str(stem) + ".bin")
    if not force and js.exists() and binf.exists():
        try:
            old = json.loads(js.read_text())
            if old.get("okey") == okey and "undef" in old and old.get("key") == link_key(okey, old["undef"], syms):
                old["cached"] = True
                return old
        except ValueError:
            pass
    subprocess.run([CC, *CFLAGS, str(src), "-o", str(stem) + ".o"], check=True)
    nm = subprocess.run(["arm-none-eabi-nm", str(stem) + ".o"], capture_output=True, text=True, check=True).stdout
    funcs = [l.split()[-1] for l in nm.splitlines() if re.match(r"^[0-9a-f]+ T ", l)]
    undef = sorted({l.split()[-1] for l in nm.splitlines() if re.match(r"^\s+U ", l)})
    name = re.search(r"^\s*\*\s*(\w+) -- stock", src.read_text(), re.M).group(1)
    if name not in funcs:
        sys.exit(f"{src}: header names {name} but the file defines {funcs}")
    lds = [f"ENTRY({name})", "SECTIONS {",
           f"  . = 0x{SCRATCH:08X};",
           "  .text : { *(.text*) *(.rodata*) *(.data*) *(.bss*) *(COMMON) }",
           "  /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }", "}"]
    # only the symbols the object needs: the others cannot change the image
    lds += [f"{k} = 0x{syms[k]:08X};" for k in undef if k in syms]
    (Path(str(stem) + ".ld")).write_text("\n".join(lds) + "\n")
    subprocess.run(["arm-none-eabi-ld", "-q", "-T", str(stem) + ".ld", "-o", str(stem) + ".elf", str(stem) + ".o"],
                   check=True)
    subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", str(stem) + ".elf", str(stem) + ".bin"], check=True)
    thumb_jumps = fix_thumb_calls(str(stem) + ".elf", str(stem) + ".bin", SCRATCH, syms)
    st = subprocess.run(["arm-none-eabi-nm", str(stem) + ".elf"], capture_output=True, text=True).stdout
    entry = int(next(l.split()[0] for l in st.splitlines() if l.split()[-1] == name), 16)
    info = {"orig": orig, "name": name, "entry": entry, "thumb": bool(entry & 1), "src": str(src.relative_to(ROOT)),
            "okey": okey, "undef": undef, "key": link_key(okey, undef, syms)}
    if thumb_jumps:
        info["thumb_jumps"] = thumb_jumps
    js.write_text(json.dumps(info))
    return info


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--force", action="store_true", help="ignore the cache")
    ap.add_argument("-j", type=int, default=os.cpu_count() or 1)
    a = ap.parse_args()
    syms = original_symbols()
    srcs = list(srcindex.sources().values())

    def one(s):
        try:
            return build(s, syms, a.force)
        except (subprocess.CalledProcessError, SystemExit, Exception) as e:
            return {"error": f"{s.relative_to(ROOT)}: {e}"}
    with ThreadPoolExecutor(max_workers=max(1, a.j)) as ex:
        infos = list(ex.map(one, srcs))
    errs = [i["error"] for i in infos if "error" in i]
    fresh = [i for i in infos if "error" not in i and not i.get("cached")]
    for info in fresh:
        print(f"built {info['name']:<14} orig 0x{info['orig']:08x}  -> 0x{info['entry']:08x}")
    for e in errs:
        print("FAILED", e)
    print(f"{len(srcs)} nonmatching functions: {len(fresh)} built, {len(srcs) - len(fresh) - len(errs)} up to date"
          + (f", {len(errs)} FAILED" if errs else ""))
    if errs:
        sys.exit(1)


if __name__ == "__main__":
    main()
