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
from tools.emu.tests import fatfs, fatfs_fs

ROOT = Path(__file__).resolve().parents[2]

MUTANTS = [
    (0x40056BBC, "r = *d++ - *s++;", "r = *s++ - *d++;"),             # mem_cmp sign
    (0x40056BBC, "while (--cnt && r == 0)", "while (--cnt)"),          # mem_cmp early exit
    (0x40056A80, "rv = rv << 8 | ptr[0];", "rv = rv << 8 | ptr[1];"),  # ld_dword byte
    (0x40056CF0, "fs->n_fats == 2", "fs->n_fats >= 1"),                # sync_window 2nd FAT
    (0x40056D80, "sector = 0xFFFFFFFF;", "sector = 0;"),               # move_window invalidate
    (0x40059454, "return 3;", "return 2;"),                            # check_fs code
    (0x40059454, '"FAT32", 5', '"FAT32", 4'),                          # check_fs length
    (0x40056C84, "if (szb < 2) return 0;\n\t\t*buf++ = (char)(wc >> 8);",
     "if (szb < 1) return 0;\n\t\t*buf++ = (char)(wc >> 8);"),  # put_utf bound (ANSI branch)
    (0x40056B90, "if (cnt != 0) {", "if (cnt > 1) {"),                 # mem_cpy count
    # filesystem layer (scenario tests on formatted volumes)
    (0x40056ED4, "wc = fs->win[bc++ % SS(fs)];", "wc = fs->win[bc % SS(fs)];"),          # get_fat FAT12
    (0x40056EA8, "clst -= 2;", "clst -= 1;"),                                             # clst2sect
    (0x400576B0, "if (ncl == 0) {", "if (ncl == 1) {"),                                   # create_chain
    (0x40059F7C, "if (btr > remain) btr = (UINT)remain;", "if (btr > remain + 1) btr = (UINT)remain + 1;"),  # f_read EOF clamp
    (0x400579D4, "if (ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? MAX_DIR_EX : MAX_DIR) || ofs % SZDIRE) {",
     "if (ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? MAX_DIR_EX : MAX_DIR)) {"),  # dir_sdi alignment
    (0x40058F8C, "if (di >= FF_MAX_LFN) return FR_INVALID_NAME;", "if (di >= 250) return FR_INVALID_NAME;"),  # create_name length
    (0x4005A6CC, "if (ofs > fp->obj.objsize && (FF_FS_READONLY || !(fp->flag & FA_WRITE))) {",
     "if (ofs > fp->obj.objsize + 1 && (FF_FS_READONLY || !(fp->flag & FA_WRITE))) {"),  # f_lseek clamp
    (0x40058094, "sum = (sum >> 1) + (sum << 7) + *dir++;", "sum = (sum >> 1) + (sum << 6) + *dir++;"),  # sum_sfn
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
            suite = fatfs.TESTS if orig in fatfs.TESTS else fatfs_fs.TESTS
            _, fails, _ = run_one(orig, suite[orig], 300 if suite is fatfs.TESTS else 40, 7)
            ok = bool(fails)
            caught += ok
            print(f"{'caught ' if ok else 'MISSED '} 0x{orig:08x}: {old!r} -> {new!r}")
    for orig, _, _ in MUTANTS:                       # restore genuine builds
        nmbuild.build(next(ROOT.glob(f"nonmatching/**/func_{orig:08x}.c")), syms)
    print(f"{caught}/{len(MUTANTS)} mutants detected")
    sys.exit(0 if caught == len(MUTANTS) else 1)


if __name__ == "__main__":
    main()
