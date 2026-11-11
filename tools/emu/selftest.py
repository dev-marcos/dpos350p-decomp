#!/usr/bin/env python3
"""Mutation self-test for the equivalence harness.

Injects small, realistic bugs into nonmatching C files (in a temporary copy), rebuilds them
and checks that tools/emu reports DIFFERENT for every mutant. Restores the real builds after.
Usage (inside dpos-re):  python3 -m tools.emu.selftest
"""
import random
import sys
import tempfile
from pathlib import Path

from tools import nmbuild
from tools.emu.run_tests import run_one
from tools.emu.tests import fatfs

ROOT = Path(__file__).resolve().parents[2]

MUTANTS = [
    (0x40056BBC, "r = *d++ - *s++;", "r = *s++ - *d++;"),             # mem_cmp sign
    (0x40056BBC, "while (--cnt && r == 0)", "while (--cnt)"),          # mem_cmp early exit
    (0x40056A80, "rv = rv << 8 | ptr[0];", "rv = rv << 8 | ptr[1];"),  # ld_dword byte
    (0x40056CF0, "fs->n_fats == 2", "fs->n_fats >= 1"),                # sync_window 2nd FAT
    (0x40056D80, "sector = 0xFFFFFFFF;", "sector = 0;"),               # move_window invalidate
    (0x40059454, "return 3;", "return 2;"),                            # check_fs code
    (0x40059454, '"FAT32", 5', '"FAT32", 4'),                          # check_fs length
    (0x40056C84, "if (szb < 2) return 0;", "if (szb < 1) return 0;"),  # put_utf bound
    (0x40056B90, "if (cnt != 0) {", "if (cnt > 1) {"),                 # mem_cpy count
]


def main():
    syms = nmbuild.original_symbols()
    caught = 0
    with tempfile.TemporaryDirectory(dir=ROOT / "build") as tmp:
        for orig, old, new in MUTANTS:
            src = next(ROOT.glob(f"nonmatching/**/func_{orig:08x}.c"))
            text = src.read_text()
            assert old in text, (src, old)
            mut = Path(tmp) / src.name
            mut.write_text(text.replace(old, new, 1))
            nmbuild.build(mut, syms)
            _, fails = run_one(orig, fatfs.TESTS[orig], 300, 7)
            ok = bool(fails)
            caught += ok
            print(f"{'caught ' if ok else 'MISSED '} 0x{orig:08x}: {old!r} -> {new!r}")
    for orig, _, _ in MUTANTS:                       # restore genuine builds
        nmbuild.build(next(ROOT.glob(f"nonmatching/**/func_{orig:08x}.c")), syms)
    print(f"{caught}/{len(MUTANTS)} mutants detected")
    sys.exit(0 if caught == len(MUTANTS) else 1)


if __name__ == "__main__":
    main()
