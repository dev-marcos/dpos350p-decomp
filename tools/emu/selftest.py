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
from tools.emu.tests import fatfs, fatfs_fs, rtlib, drivers

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
    # run-time library (AEABI helpers, conversions, libm; bit-exact comparisons)
    (0x4003453C, "if (sn) o->r = 0 - o->r;", "if (sd) o->r = 0 - o->r;"),  # ldivmod remainder sign
    (0x4006A6FC, "if (n & 2) { *h++", "if (n & 4) { *h++"),                 # memset4 tail
    (0x4003DF94, "if (e == 0x7FF) return 0;", "if (e == 0x7FF) return 1;"), # d2lz Inf/NaN
    (0x4006B86C, "if (k < e) return mk(hi - (k << 20), lo);",
     "if (k <= e) return mk(hi - (k << 20), lo);"),                       # ldexp normal/subnormal boundary
    (0x40068F08, "if (rh2 >= 0x79200000u)", "if (rh2 >= 0x7C200000u)"),     # exp: tiny-r shortcut bound
    (0x4006A31C, "t = t - LN2LO * kd;", "t = t + LN2LO * kd;"),             # log recombination
    (0x4005F91C, "if (half) ax = ax * 0.5;", "if (half) ax = ax * 0.25;"),  # tanh half argument
    (0x4006613C, "if (q & 2) r = -r;", "if (q & 1) r = -r;"),               # sincos sign
    (0x4005D4A4, "if (e == 1 && m < K(0x3FE6A09E667F3BCD)) {", "if (e == 2 && m < K(0x3FE6A09E667F3BCD)) {"),  # pow reduction
    (0x4005D4A4, "neg = !(yi == ye);", "neg = (yi == ye);"),                 # pow odd-power sign
    (0x40069FD0, "hq = hq * r;", "hq = hq * z;"),                            # __pow_core rational term
    # drivers (ordered MMIO access log)
    (0x400631D4, "port->cfg[idx] | (1u << sh);", "port->cfg[idx] | (2u << sh);"),  # pio_cfg_pin func 1
    (0x4004927C, "pio_set_bit(pe, 3);", "pio_clr_bit(pe, 3);"),               # FPGA command vs data strobe
    (0x40049490, "u32 n = 50000;", "u32 n = 5;"),                             # GT911 ACK timeout
    (0x4005F55C, "u32 p1 = f + (a << 7) - 512;", "u32 p1 = f + (a << 7) - 511;"),  # MS5351 P1 encoding
    (0x4006750C, "readl_b(WDOG0_MODE) | 1);", "readl_b(WDOG0_MODE) | 2);"),   # watchdog enable bit
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
            suite = next(s for s in (fatfs.TESTS, fatfs_fs.TESTS, rtlib.TESTS, drivers.TESTS) if orig in s)
            _, fails, _ = run_one(orig, suite[orig], 40 if suite is fatfs_fs.TESTS else (80 if suite is drivers.TESTS else 400), 7)
            ok = bool(fails)
            caught += ok
            print(f"{'caught ' if ok else 'MISSED '} 0x{orig:08x}: {old!r} -> {new!r}")
    for orig, _, _ in MUTANTS:                       # restore genuine builds
        nmbuild.build(next(ROOT.glob(f"nonmatching/**/func_{orig:08x}.c")), syms)
    print(f"{caught}/{len(MUTANTS)} mutants detected")
    sys.exit(0 if caught == len(MUTANTS) else 1)


if __name__ == "__main__":
    main()
