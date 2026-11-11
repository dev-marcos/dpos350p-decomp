#!/usr/bin/env python3
"""Build every nonmatching/**/func_XXXXXXXX.c as a standalone test image.

Each C file is compiled with GCC (only for behavioural checking, see docs/COMPILER.md §6)
and linked alone at SCRATCH (0x48000000). Every symbol it references but does not define
resolves to the ORIGINAL address from symbols/symbols_2025-06-19.csv, so a test exercises
exactly one rewritten function against the stock callees.

Output per function: build/nonmatching/func_XXXXXXXX.{o,elf,bin,json}
  json = {"orig": addr, "name": sym, "entry": addr_in_scratch, "thumb": bool}
"""
import csv
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRATCH = 0x48000000
CC = "arm-none-eabi-gcc"
CFLAGS = ["-c", "-marm", "-mcpu=cortex-a7", "-mfpu=neon-vfpv4", "-mfloat-abi=hard", "-O2",
          "-ffreestanding", "-fno-builtin", "-fno-common", "-ffunction-sections", "-fdata-sections",
          "-Wall", "-Wno-unused", "-I", str(ROOT / "include")]
OUT = ROOT / "build/nonmatching"


def original_symbols():
    syms = {}
    for r in csv.DictReader(open(ROOT / "symbols/symbols_2025-06-19.csv")):
        if r["type"].startswith("function"):
            a = int(r["address"], 16)
            if r["type"] == "function_thumb":
                a |= 1
            syms[r["name"]] = a
    # curated names may not be in the generated CSV yet (if `make map` was not re-run)
    for r in csv.DictReader(open(ROOT / "symbols/manual.csv")):
        if r["type"] == "function":
            syms.setdefault(r["name"], int(r["address"], 16))
    return syms


def build(src, syms):
    m = re.search(r"func_([0-9a-f]{8})\.c$", src.name)
    orig = int(m.group(1), 16)
    stem = OUT / f"func_{orig:08x}"
    OUT.mkdir(parents=True, exist_ok=True)
    subprocess.run([CC, *CFLAGS, str(src), "-o", str(stem) + ".o"], check=True)
    nm = subprocess.run(["arm-none-eabi-nm", str(stem) + ".o"], capture_output=True, text=True, check=True).stdout
    defined = {l.split()[-1] for l in nm.splitlines() if re.match(r"^[0-9a-f]+ [TtDdRrBb] ", l)}
    funcs = [l.split()[-1] for l in nm.splitlines() if re.match(r"^[0-9a-f]+ T ", l)]
    name = re.search(r"^\s*\*\s*(\w+) -- stock", src.read_text(), re.M).group(1)
    if name not in funcs:
        sys.exit(f"{src}: header names {name} but the file defines {funcs}")
    lds = [f"ENTRY({name})", "SECTIONS {",
           f"  . = 0x{SCRATCH:08X};",
           "  .text : { *(.text*) *(.rodata*) *(.data*) *(.bss*) *(COMMON) }",
           "  /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }", "}"]
    lds += [f"{k} = 0x{v:08X};" for k, v in sorted(syms.items()) if k not in defined]
    (Path(str(stem) + ".ld")).write_text("\n".join(lds) + "\n")
    subprocess.run(["arm-none-eabi-ld", "-T", str(stem) + ".ld", "-o", str(stem) + ".elf", str(stem) + ".o"],
                   check=True)
    subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", str(stem) + ".elf", str(stem) + ".bin"], check=True)
    st = subprocess.run(["arm-none-eabi-nm", str(stem) + ".elf"], capture_output=True, text=True).stdout
    entry = int(next(l.split()[0] for l in st.splitlines() if l.split()[-1] == name), 16)
    info = {"orig": orig, "name": name, "entry": entry, "thumb": bool(entry & 1), "src": str(src.relative_to(ROOT))}
    Path(str(stem) + ".json").write_text(json.dumps(info))
    return info


def main():
    syms = original_symbols()
    srcs = sorted(ROOT.glob("nonmatching/**/func_*.c"))
    for s in srcs:
        info = build(s, syms)
        print(f"built {info['name']:<14} orig 0x{info['orig']:08x}  -> 0x{info['entry']:08x}")
    print(f"{len(srcs)} nonmatching functions built")


if __name__ == "__main__":
    main()
