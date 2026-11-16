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
    # batch 13: generator, calibration selector, system calibration
    (0x4004C544, "base = SYS8(0x100) == 0 ? 16 : 0;", "base = SYS8(0x100) == 0 ? 15 : 0;"),  # column block
    (0x4004C6B4, "if (++r >= 11)", "if (++r >= 10)"),                               # POST rounds before save
    (0x4004C794, "(v * 100) / total < min_pct", "(v * 10) / total < min_pct"),      # percent scale
    (0x4004CF34, "case 8: case 10: case 14: lo = 3000000;", "case 8: case 10: case 14: lo = 3000001;"),  # 3 MHz cap
    (0x4004CF34, "else if (off < -lim) off = -lim;", "else if (off < -lim) off = lim;"),  # negative offset limit
    (0x4004D09C, "*p = e[1] - *p;", "*p = e[1] + *p;"),                             # period inversion
    (0x4004D390, "t = t - hd;", "t = t - hd - 1.0;"),                               # normalisation shift
    (0x4004D1B4, "v = fr != 0 ? (u32)g[0x60 / 4] / fr : mx;", "v = fr != 0 ? (u32)g[0x60 / 4] / (fr + 1) : mx;"),  # period length
    (0x4004D49C, "GEN_BUF[0] = GEN_BUF[1];", "GEN_BUF[0] = GEN_BUF[2];"),           # resample edge repeat
    # batch 20: settings records, SPI flash helpers
    (0x400439A0, "for (i = 0; i < 0x40; i++)", "for (i = 0; i < 0x3F; i++)"),
    (0x40043CD0, "serializer_process_field(d + 0x960, 8);", "serializer_process_field(d + 0x958, 8);"),
    (0x40043D54, "SER_IMAGE, 0x2000);", "SER_IMAGE, 0x1000);"),
    (0x40043AB4, "if (SYS32(0) != old) {", "if (SYS32(0) == old) {"),
    (0x40043E34, "for (i = 1; i <= 5; i++) {", "for (i = 1; i < 5; i++) {"),
    (0x40043E34, "== 0x5A5555A5u)", "== 0x5A5555A4u)"),
    (0x4004341C, "pio_set_drive((void *)0x01C20848u, pin, 3);", "pio_set_drive((void *)0x01C20848u, pin, 2);"),
    (0x40043494, "sys_spi_write_then_read(dev, &c, 1, &rx, 1);", "sys_spi_write_then_read(dev, &c, 1, &rx, 2);"),
    (0x400434F4, "while (spi_flash_cmd_read1(dev, 5) & 1)", "while (spi_flash_cmd_read1(dev, 5) & 2)"),
    (0x40043514, "== 1 ? 0 : -1;", "!= 0 ? 0 : -1;"),
    # batch 19: channel filters
    (0x40040F18, "if (i >= w - 1) {", "if (i >= w) {"),
    (0x40040F18, "if (w < 2)", "if (w < 3)"),
    (0x40040FC4, "passes = vd == 12 ? 6 : 3;", "passes = vd == 12 ? 6 : 2;"),
    (0x40040FC4, "if (d < -lim) d = -lim;", "if (d < -lim) d = lim;"),
    (0x40041094, "SYS32(0x454 + 4 * k));", "SYS32(0x450 + 4 * k));"),
    (0x40041118, "if (d < dev[k]) d = dev[k];", "if (d > dev[k]) d = dev[k];"),
    (0x40041118, "if (lim <= dev[k])", "if (lim < dev[k] - 1)"),
    (0x40041118, "init = (int)((1.0 - (double)fr) * (double)(float)lim);", "init = (int)((1.0 - (double)fr) * (double)(float)lim) + 1;"),
    (0x40041324, "} else if (tb < 21) {", "} else if (tb < 20) {"),
    # batch 18: CSI bank unpacking
    (0x4004073C, "q = (int)((u32)gain * (u32)cg) / 1000;", "q = (int)((u32)gain * (u32)cg) / 1024;"),
    (0x4004073C, "v = (int)(base + (u32)(v >> 5));", "v = (int)(base + (u32)(v >> 4));"),
    (0x4004073C, "            v = top;", "            v = top - 1;"),
    (0x4004073C, "(u32)q * SYS8(0x1B8)", "(u32)q * SYS8(0x1B9)"),
    (0x4004073C, "buf += stride;", "buf += nb;"),
    (0x40040E44, "SYS8(0x6D4 + i), SYS16(0x6F4)", "SYS8(0x6D3 + i), SYS16(0x6F4)"),
    (0x400408FC, "ch, buf, 8, 7 - i,", "ch, buf, 8, 8 - i,"),
    # batch 17: acquisition pipeline
    (0x40040048, "* 1.3));", "* 1.25));"),
    (0x4004006C, "if (SYS32(0x554) >= SYS32(0x55C))", "if (SYS32(0x554) > SYS32(0x55C))"),
    (0x4004006C, "w >> 1, w & 0xFFFF", "w >> 2, w & 0xFFFF"),
    (0x400405E0, "w = 0x14 + 2u *", "w = 0x12 + 2u *"),
    (0x400406C8, "(int)(short)i * 0x2134", "(int)(unsigned short)i * 0x2134"),
    (0x4004071C, "for (i = 4; i < 8; i++)", "for (i = 4; i < 7; i++)"),
    (0x400417E4, "} else if (m == 2) {", "} else if (m == 1) {"),
    (0x400417E4, "+ 0x62250), (const", "+ 0x62200), (const"),
    (0x4004191C, "mode != 5 && mode != 0x13", "mode != 5 && mode != 0x12"),
    (0x4004191C, "while (SYS32(0x700) < SYS16(8))", "while (SYS32(0x700) <= SYS16(8))"),
    (0x400419CC, "if (SYS8(0x28) == 0)", "if (SYS8(0x28) != 0)"),
    # batch 16: touch controls, complex helpers, LRADC, strcmp, USB disk geometry
    (0x400690E4, "if (TS32(0x74) < d1) d1 = 0xFFFF;", "if (TS32(0x74) <= d1) d1 = 0xFFFF;"),
    (0x40069234, "int a = (int)(*(volatile u32 *)(b + 0xAC64) + o);", "int a = (unsigned short)(*(volatile u32 *)(b + 0xAC64) + o);"),
    (0x40069150, "if (TS32(0x70) < best) {", "if (TS32(0x70) <= best) {"),
    (0x400692A0, "if (best < TS32(0x60))", "if (best <= TS32(0x60))"),
    (0x40069384, "TP[3] < (int)(SYS16(6)", "TP[3] <= (int)(SYS16(6)"),
    (0x40069428, "return (int)(v << 1) / 3 +", "return (v << 1) / 3 +"),
    (0x40066538, "float m = re * re + im * im;", "float m = re * re + im * re;"),
    (0x40066584, "r.re = a.re * b.re - a.im * b.im;", "r.re = a.re * b.re + a.im * b.im;"),
    (0x40068E94, "writel_c(r, readl_c(r) | 0x2000u);", "writel_c(r, readl_c(r) | 0x1000u);"),
    (0x40068F78, "} while (c >= 1 && c == d);", "} while (c == d);"),
    (0x4006A98C, "if (*(volatile u32 *)0x40095490u == 1)", "if (*(volatile u32 *)0x40095490u != 0)"),
    # batch 15: serialiser, code page, strlen, USB veneers, touch helpers
    (0x40067160, "d[6] = *p >> 8;", "d[6] = *p >> 16;"),
    (0x400672A8, "for (i = 1; i < 7; i++)", "for (i = 1; i < 6; i++)"),
    (0x40067278, "SER_CURSOR = SER_CURSOR + size;", "SER_CURSOR = SER_CURSOR + 1;"),
    (0x40067450, "if (SER_MODE == 0)", "if (SER_MODE != 1)"),
    (0x40069D90, "return (c + 0x80) & 0xFF;", "return c + 0x80;"),
    (0x40069DF4, "if (oem < 0x80)", "if ((u32)oem < 0x80)"),
    (0x40069E38, "case 8: uc -= 0x1C60; break;", "case 8: uc -= 0x1C50; break;"),
    (0x40069E38, "if (cmd == 0)", "if (cmd != 1)"),
    (0x4006A964, "return a;", "return 0;"),
    (0x40069CF8, "dx /= 10;", "dx /= 8;"),
    # batch 14: acquisition control
    (0x4003F948, "u32 v = n - 2;", "u32 v = n - 1;"),                                  # sample count bias
    (0x4003FAB8, "u32 v = L + SYS32(0x63C)", "u32 v = SYS32(0x63C)"),                  # wrap term
    (0x4003FAE8, "t = __aeabi_ldivmod(t, (long long)SYS8(0x1B1));", "t = __aeabi_ldivmod(t, 1);"),
    (0x4003FAE8, "b = r4 - a;", "b = a - r4;"),
    (0x4003FB90, "while (d < 1)", "while (d < 0)"),                                    # strictly positive delay
    (0x4003FB90, "SYS32(0x63C) = 1;", "SYS32(0x63C) = 0;"),
    (0x4003FCC0, "if (++n < 2500)", "if (++n < 2499)"),
    (0x4003FCC0, "if (g_touch->busy2 != 0)", "if (g_touch->busy2 == 0)"),
    (0x4003FD10, "(int)r5 >= 23", "(int)r5 >= 24"),
    (0x4003FD10, "if (span < p)", "if (span <= p)"),
    (0x4003FD10, "< 25)", "< 24)"),
    (0x4003FA10, "fpga_selector_command_write(0x11); fpga_data_byte_write(1);", "fpga_selector_command_write(0x11); fpga_data_byte_write(0);"),
    # batch 21: UI window layer
    (0x4003CA88, "need = (w[2] == 0 ? acc : own) | d;", "need = (w[2] == 0 ? own : acc) | d;"),
    (0x4003CA88, "        w[3] = 0;\n", ""),
    (0x4003CA88, "case 13: ui_window_13_redraw(anim)", "case 13: ui_window_14_redraw(anim)"),
    (0x4003D594, "if (s < 0.01)", "if (s < 0.02)"),
    (0x4003D594, "TANH_T(t) + 0.02", "TANH_T(t) + 0.03"),
    (0x4003D46C, "(y + h - n) & 0xFFFF", "(y + h - n + 1) & 0xFFFF"),
    (0x4003CD0C, "for (i = 19; i >= 0; i--)", "for (i = 18; i >= 0; i--)"),
    (0x4003CDFC, "WIN(id)[3] = 1;", "WIN(id)[2] = 1;"),
    (0x400343C4, "u32 n = x1 - x0 + 1", "u32 n = x1 - x0"),
    (0x4003DD5C, "(0x42E68A88u + 2))", "(0x42E68A88u + 3))"),
    (0x4003D8A4, "ui_reveal_bottom_up((volatile", "ui_reveal_slide_down((volatile"),
    # batch 22: trace chain, window stack helpers
    (0x4003C550, "if (SYS8(0x2AA))\n            func_4005e10c(3);", "if (SYS8(0x2AA))\n            func_4005e10c(2);"),
    (0x4003C7A0, "if (SYS32(0x6FC) < SYS32(0x708))", "if (SYS32(0x6FC) <= SYS32(0x708))"),
    (0x4003C7E4, "if (k == 1 && SYS8(0x2AA) == 0)", "if (k == 0 && SYS8(0x2AA) == 0)"),
    (0x4003C8D4, "if (p[0x664] == 0)", "if (p[0x664] != 0)"),
    (0x4003C970, "SYS8(0x3C) == 0 && SYS8(0x38) == 0", "SYS8(0x3C) == 0"),
    (0x4003CA38, "d[s[1]] = s[0];", "d[s[0]] = s[1];"),
    (0x4003CD44, "slot &= 0xFF;", "slot &= 0x7F;"),
    (0x4003CE50, "if (slot < 19)", "if (slot < 18)"),
    (0x4003CE50, "        if (k)\n", "        if (k || 1)\n"),
    (0x4003CF58, "        return;\n    fb_present();", "        fb_present();\n    fb_present();"),
    # batch 23: slots, calibration/G/H records, zoom index
    (0x4004435C, "return s <= 4 ? SYS32(0x7C8 + 4 * s) : 0;", "return s <= 4 ? SYS32(0x7C4 + 4 * s) : 0;"),
    (0x40044680, "SER_IMAGE, 0x5000", "SER_IMAGE, 0x1000"),
    (0x4004480C, "serializer_process_field(&SYS32(0x764), 4);", "serializer_process_field(&SYS32(0x760), 4);"),
    (0x4003BE50, "SYS16(0x870) = 1;", "SYS16(0x872) = 1;"),
    (0x40041F5C, "SYS8(0x2AA) || SYS8(0x28) || SYS8(0x652)", "SYS8(0x2AA) || SYS8(0x28)"),
    (0x40041FF8, "SYS32(0xBD0 + 8 * SYS8(0x598))", "SYS32(0xBD0 + 4 * SYS8(0x598))"),
    (0x40042350, "SYS8(0x598) = lim - 1;", "SYS8(0x598) = lim - 2;"),
    (0x40042350, "if (SYS8(0x28) && SYS8(0x3C) == 0)", "if (SYS8(0x28))"),
    (0x40042470, "if (d < b)", "if (d <= b)"),
    (0x400424D4, "zoom_index_set((SYS8(0x5B0) - 1) & 0xFF);", "zoom_index_set(SYS8(0x5B0));"),
    # batch 24: display bring-up, defaults, state init, SPI erase/write
    (0x4003E63C, "if (pin == 12)", "if (pin == 11)"),
    (0x4003ED28, "W(0x2F08) = 500;", "W(0x2F08) = 1000;"),
    (0x4003ED80, "W(0xAC64) = h / 5u;", "W(0xAC64) = h / 4u;"),
    (0x4003EDF4, "W(0x5C) = v / 5;", "W(0x5C) = (u32)v / 5u;"),
    (0x4003EF1C, "SYS8(0xBC9) = ~SYS8(0x105);", "SYS8(0xBC9) = ~SYS8(0x41);"),
    (0x4003EFAC, "memset_w((void *)0x42E68BCAu, 0, 0x14);", "memset_w((void *)0x42E68BC8u, 0, 0x14);"),
    (0x4003C008, "SYS16(0x6F4) = q + 2;", "SYS16(0x6F4) = q + 1;"),
    (0x4003C008, "SYS8(0x588) == 0x21 ? 2 : 1", "SYS8(0x588) == 0x20 ? 2 : 1"),
    (0x40043564, "cmd[0] = 0x21;", "cmd[0] = 0x20;"),
    (0x4004380C, "if (n > 0x100)\n            n = 0x100;", "if (n > 0xFF)\n            n = 0xFF;"),
    # batch 25: time-base index, committed copy, acquisition glue
    (0x4003BCF0, "if (q < lim)\n            return;", "if (q <= lim)\n            return;"),
    (0x4003BCF0, "ui_window_invalidate_from(3, c);", "ui_window_invalidate_from(3, c + 1);"),
    (0x4003BCF0, "SYS8(0x588) = c + 3;", "SYS8(0x588) = c + 2;"),
    (0x4003BDE8, "SYS8(0x588) = c >= d ? c - d : 0;", "SYS8(0x588) = c >= (d & 0xFF) ? c - d : 0;"),
    (0x4003D6E4, "r1 = inv_r1_out(id, r1);", "r1 = r1;"),
    (0x4003BF98, "q = d ? c / d : 0;", "q = d ? c / d : c;"),
    (0x4003BEB4, "SYS64(0x5E0) = SYS64(0x570);", "SYS64(0x5E0) = SYS64(0x578);"),
    (0x4003C0AC, "p = (long long)(int)(SYS32(0x5EC) - 1) * k;\n        SYS32(0x624)", "p = (long long)(u32)(SYS32(0x5EC) - 1) * k;\n        SYS32(0x624)"),
    (0x4003FFE4, "2u * ((0xC738u - n) >> 1)", "(0xC738u - n)"),
    (0x4004372C, "cmd[0] = 0x12;", "cmd[0] = 0x02;"),
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
