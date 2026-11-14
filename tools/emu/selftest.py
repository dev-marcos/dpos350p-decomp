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
from tools.emu.tests import fatfs, fatfs_fs, rtlib, drivers, usb, platform, fpga

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
    (0x40042B60, "interval * 24", "interval * 25"),                         # timer interval scale
    (0x400427B0, "if (f >= 0x2DC6C001u)", "if (f > 0x2DC6C001u)"),            # PLL_CPU P=1 threshold
    (0x400428AC, "((v >> 4) & 3) + 1", "((v >> 4) & 1) + 1"),                # PLL_PERIPH0 K field
    (0x40042978, "~(1u << (code & 0x1F)) & readl_e(r)", "~(1u << (code & 0xF)) & readl_e(r)"),  # reset-bit field
    # USB stack (EasyUSB-derived + FNIRSI mass storage)
    (0x4006ADBC, "usb_ep0_state = (d[0] & 0x80) ? 1 : 2;", "usb_ep0_state = (d[0] & 0x40) ? 1 : 2;"),  # ep0 data direction
    (0x4006BA98, "if (max_lun < CBW_LUN", "if (max_lun <= CBW_LUN"),                    # CBW LUN bound
    (0x4006DCE4, "if (lun_blocks[lun] < lba + count)", "if (lun_blocks[lun] <= lba + count)"),  # READ/WRITE range
    (0x4006D898, "if ((int)n >= 0x25) n = 0x24;", "if ((int)n >= 0x25) n = 0x23;"),    # INQUIRY length clamp
    (0x4006C264, "v |= ((ep_index - 1) << 1) << 1 | 1;", "v |= ((ep_index - 1) << 1) | 1;"),  # DMA DRQ select
    (0x4006C9A8, "u32 temp = ((fifo_size + 511) & ~511u) >> 4", "u32 temp = ((fifo_size + 511) & ~511u) >> 3"),  # FIFO size code
    (0x4006C08C, "mass_stage = next ? 4 : 5;", "mass_stage = next ? 5 : 4;"),          # CSW next stage
    (0x4006AA28, "if (len > ep_max_len[ep]) {", "if (len >= ep_max_len[ep]) {"),       # short/long IN split
    # platform drivers (GIC, UART/printf, SPI, CSI, SD/MMC)
    (0x40066D88, "len == 3 || len == 7", "len == 4 || len == 7"),              # printf thousands separator
    (0x40066950, "+ 0x1100;", "+ 0x1104;"),                                    # GIC ISENABLER offset
    (0x40066C90, "& 2))", "& 4))"),                                            # UART TX-not-full bit
    (0x40043164, "if (txn >= 32) {", "if (txn > 32) {"),                       # SPI word/byte TX split
    (0x40065954, "cfg[3] == 1 ? 0x10000 : 0", "cfg[3] == 2 ? 0x10000 : 0"),    # CSI clock edge
    (0x400677E4, "if (hz < 25000001) src |= 0x500000;", "if (hz < 25000001) src |= 0x400000;"),  # SD clock phase
    (0x40067B30, "if (cnt >= 0x101)", "if (cnt >= 0x100)"),                    # IDMAC descriptor limit
    (0x40067D70, "if (data->blksz * data->blocks > 0x40) {", "if (data->blksz * data->blocks >= 0x40) {"),  # CPU vs DMA data path
    (0x40068918, "if (cmd.resp[3] & 0x40000000u) {", "if (cmd.resp[3] & 0x20000000u) {"),  # SDHC (CCS) detection
    (0x4006866C, "if (mmc_card_type != 1) sector <<= 9;", "if (mmc_card_type != 1) sector <<= 8;"),  # byte addressing
    # display (TCON0 / DE2) and framebuffer helpers
    (0x4003E064, "if (t[0x2C / 4] == 0) v |= 0x08000000;", "if (t[0x2C / 4] == 0) v |= 0x00800000;"),  # TCON IO polarity
    (0x4006301C, "~(0xFu << sh)", "~(0x7u << sh)"),                              # DE2 blender route mask
    (0x40063070, "de_writel(r + 4, 0xFFFEFFFFu + (w + (h << 16)));", "de_writel(r + 4, 0xFFFFFFFFu + (w + (h << 16)));"),  # UI layer size
    (0x400356F4, "while (n) { *d++ = v; n--; }", "while (n > 1) { *d++ = v; n--; }"),  # fill count
    # batch 11: FPGA / SMEC / touch / power / disk glue
    (0x4004A0B0, "if (v <= 3000)", "if (v < 3000)"),                                # SMEC timing ceiling
    (0x4004A0B0, "if (tries >= 4)", "if (tries >= 3)"),                             # SMEC retries
    (0x40049F7C, "fpga_data_byte_write((v << 16) >> 24);", "fpga_data_byte_write((v << 16) >> 23);"),  # job value byte 2
    (0x40049EC4, "s->valid[idx] == 0", "s->valid[idx] == 1"),                       # parameter cache
    (0x400498E4, "if (x > 0x3FE) x = 0x3FE;", "if (x > 0x3FF) x = 0x3FF;"),         # touch x clamp
    (0x400498E4, "g_touch_last + 15 < g_tick_count", "g_touch_last + 14 < g_tick_count"),  # poll period
    (0x40049A9C, "t->busy2 = 0;", "t->busy = 0;"),                                  # touch enable gate
    (0x40049C78, "res = 1;", "res = 2;"),                                           # disk_read error code
    (0x40049CE8, "return 4;", "return 1;"),                                         # disk_write wrong drive
    (0x4004A1F4, "SYS32(0x65C) < g_tick_count", "SYS32(0x65C) <= g_tick_count"),    # auto power-off deadline
    # batch 12: battery, front end, POST
    (0x4004A2D0, "else if (mv >= 3970) lv = 8;", "else if (mv >= 3980) lv = 8;"),  # battery threshold
    (0x4004A448, "g_batt_last >= 1001", "g_batt_last >= 1000"),                                             # battery average period
    (0x4004A608, "p1 == 1 ? 10 : 100", "p1 == 1 ? 10 : 10"),                        # probe x100
    (0x4004A7C4, "int lo = m * 2", "int lo = m * 3"),                               # position margin
    (0x4004A884, "s[n] = 'm';", "s[n] = 'u';"),                                     # mV label
    (0x4004ACC8, "if ((int)dhi < 0) dlo = 0;", "if ((int)dhi < -1) dlo = 0;"),     # negative terms clipped
    (0x4004AF0C, "u32 tol = vdiv == 12 ? 6 : 3;", "u32 tol = vdiv == 12 ? 6 : 4;"), # POST tolerance
    (0x4004AAC0, "if (v == 12) v = 11;", "if (v == 12) v = 12;"),                   # V/div 12 -> 11
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
            suite = next(s for s in (fatfs.TESTS, fatfs_fs.TESTS, rtlib.TESTS, drivers.TESTS, usb.TESTS, platform.TESTS, fpga.TESTS) if orig in s)
            _, fails, _ = run_one(orig, suite[orig], 40 if suite is fatfs_fs.TESTS else (80 if suite in (drivers.TESTS, usb.TESTS, platform.TESTS, fpga.TESTS) else 400), 7)
            ok = bool(fails)
            caught += ok
            print(f"{'caught ' if ok else 'MISSED '} 0x{orig:08x}: {old!r} -> {new!r}")
    for orig, _, _ in MUTANTS:                       # restore genuine builds
        nmbuild.build(next(ROOT.glob(f"nonmatching/**/func_{orig:08x}.c")), syms)
    print(f"{caught}/{len(MUTANTS)} mutants detected")
    sys.exit(0 if caught == len(MUTANTS) else 1)


if __name__ == "__main__":
    main()
