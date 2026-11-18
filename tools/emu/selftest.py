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
from tools.emu.tests import fatfs, fatfs_fs, rtlib, drivers, usb, platform, fpga, sweep

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
    (0x4003C550, "if (SYS8(0x2AA))\n            layer_clear(3);", "if (SYS8(0x2AA))\n            layer_clear(2);"),
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
    # batch 26: settings image sections, UI text table, entry pad
    (0x40045580, 'for (i = 0; i < n; i++)', 'for (i = 0; i <= n; i++)'),
    (0x400455E4, 'serializer_process_field((volatile void *)(0x42B29F44u + 2 * i), 2);', 'serializer_process_field((volatile void *)(0x42B29F46u + 2 * i), 2);'),
    (0x40045714, 'SYS32(0x76C) = serializer_get_cursor() - SYS32(0x760);', 'SYS32(0x76C) = serializer_get_cursor() - SYS32(0x75C);'),
    (0x400342B4, 'while (*s++);', 'while (*++s);'),
    (0x400457B0, 'if (L < 1 || L > 4)', 'if (L < 1 || L > 3)'),
    (0x400457B0, 'TB(e + 0x195) = TB(e + 0x197 + 2 * L);', 'TB(e + 0x195) = TB(e + 0x196 + 2 * L);'),
    (0x400458FC, 'TB(e + 0x19B) = b;', 'TB(e + 0x19B) = a;'),
    (0x4005FBB4, "else if (c == '.')", "else if (c == ',')"),
    (0x4005FB4C, '*(volatile u32 *)0x42E68AB4u = 0x42E68AC8u;', '*(volatile u32 *)0x42E68AB4u = 0x42E68AC0u;'),
    (0x4005F8E8, 'trace_scale_segment(tr, SYS32(0x6FC), SYS16(0x6F6));', 'trace_scale_segment(tr, SYS32(0x6F8), SYS16(0x6F6));'),
    # batch 27: touch buttons, window touch handling, loose helpers
    (0x4002F2BC, 'if (!(x < (int)*(volatile unsigned short *)(b + 6)))', 'if (!(x <= (int)*(volatile unsigned short *)(b + 6)))'),
    (0x4002F2BC, 'if (TCH8(0xC) == 2)\n        b[0] = 2;', 'if (TCH8(0xC) == 3)\n        b[0] = 2;'),
    (0x4002F194, '                    if (a >= y)\n                        return;', '                    if (a > y)\n                        return;'),
    (0x4002F194, 'ui_window_close(id, 0, (u32)r2);', 'ui_window_close(id, 0, (u32)x);'),
    (0x4002F228, 'if (st == 2 && p[2] == 0) {', 'if (st != 1 && p[2] == 0) {'),
    (0x4002F4F0, '    touch_button_update((volatile unsigned char *)0x42E6227Au);\n', ''),
    (0x4005FC04, 'for (i = 0x15; i >= 3; i--)', 'for (i = 0x14; i >= 3; i--)'),
    (0x4005D220, 'return -SINT[n % 20001];', 'return -SINT[n % 20000];'),
    (0x4005D27C, 'a = (double)i * 0.000314159265359;', 'a = (double)(i + 1) * 0.000314159265359;'),
    (0x40060D68, 'CB32(0xF4) = old & 0xFF;', 'CB32(0xF4) = old;'),
    # batch 28: small leftovers (setters, gates, FPGA commands, drawing wrappers)
    (0x4002EB70, 'y * 1022u', 'y * 1023u'),
    (0x4002EBA8, '(c - b + 1) & 0xFFFF', '(c - b) & 0xFFFF'),
    (0x400301A8, 'if (mode == 1)', 'if (mode != 0)'),
    (0x400301A8, 'for (n = 24; n; n--)', 'for (n = 23; n; n--)'),
    (0x4006366C, 'SYS8(0x275) && SYS8(0x274) && SYS8(0x27A)', 'SYS8(0x275) && SYS8(0x27A)'),
    (0x400636BC, '(SYS32(0x288) >> 8) & 0xFF', '(SYS32(0x288) >> 16) & 0xFF'),
    (0x40064DC0, 'r = gen_selftest_64cf0() & a & 1;', 'r = gen_selftest_64cf0() & 1;'),
    (0x4002D6D4, 'e & 0xFF, 1)', 'e, 1)'),
    (0x40024E14, 'if (SYS8(0x652) == 0)', 'if (SYS8(0x652) != 1)'),
    (0x40034284, '(h[3] + y + 1)', '(h[3] + x + 1)'),
    # batch 29: colour, buffer copy, list at SYS+0xFA0, clamps, labels, file checks
    (0x4002EA38, '((c >> 19) << 11)', '((c >> 18) << 11)'),
    (0x4003436C, 'for (y = y0; y < y1; y++)', 'for (y = y0; y <= y1; y++)'),
    (0x40036D88, '(int)SYS16(0x758) - 1; k >= 1', '(int)SYS16(0x758) - 0; k >= 1'),
    (0x400353A8, 'if (*p++)', 'if (*p++ > 1)'),
    (0x40035248, 'if (m < v)\n            SS16(0xF9E) = m;', 'if (m <= v)\n            SS16(0xF9E) = m + 1;'),
    (0x40037418, 'if (i < SS16(0xFA0) - 1)', 'if (i <= SS16(0xFA0) - 1)'),
    (0x400374EC, 's += n;', 's += n - 1;'),
    (0x400340D4, 'if (v < 12) {\n        SYS32(0x814) = 12;', 'if (v < 11) {\n        SYS32(0x814) = 12;'),
    (0x40037A60, 'if (lim <= x)', 'if (lim < x - 1)'),
    (0x40038DEC, 'c & 0xFF, v);', 'c & 0xFF, -v);'),
    (0x400353F4, 'if (f_open(fil, (const char *)0x4054D728u, 1) == 0) {', 'if (f_open(fil, (const char *)0x4054D728u, 1) != 1) {'),
    (0x40062EE8, 'if (CB8(0) == 0 || CB8(4) == 0) {', 'if (CB8(0) == 0 && CB8(4) == 0) {'),
    # batch 30: file lists A, B, C
    (0x400352A8, 'if (lim > end)', 'if (lim >= end + 1 || lim < 0)'),
    (0x400352A8, 'lim - start, 1);', 'lim - start + 1, 1);'),
    (0x40035320, '/ (int)SYS16(0xF9A) + 1; i++)', '/ (int)SYS16(0xF9A); i++)'),
    (0x4003486C, 'if (c - ps * q)\n        q++;', 'if (c - ps * q > 0)\n        q++;'),
    (0x4003486C, 'f_lseek(fil, 2);', 'f_lseek(fil, 0);'),
    (0x40034920, 'SYS16(0x754) * 2u, 0);', 'SYS16(0x754) * 2u + 2, 0);'),
    (0x40035434, 'for (i = SS16(0xFA0) - 1; i >= 0; i--) {', 'for (i = SS16(0xFA0) - 1; i > 0; i--) {'),
    (0x400354C0, '        if (v == 0)\n            break;\n', ''),
    (0x400350F4, 'if (SYS8(0x5DC6 + i)) {', 'if (SYS8(0x5DC6 + i) & 1) {'),
    (0x40036C54, 'f_lseek(fil, 4);', 'f_lseek(fil, 6);'),
    (0x40036BDC, 'SYS32(0x3DA544) * SYS16(0x758), 0);', 'SYS32(0x3DA544) * SYS16(0x756), 0);'),
    (0x400364E0, 'if (f_open(fil, PATHBUF, 1) == 0)', 'if (f_open(fil, PATHBUF, 3) == 0)'),
    # batch 31: file lists, second part
    (0x40034A0C, 'SYS16(0xFA6 + 2 * k) = SYS16(0xFA8 + 2 * k);', 'SYS16(0xFA6 + 2 * k) = SYS16(0xFAA + 2 * k);'),
    (0x40036F74, '    if (c)\n        SS16(0xFA0) = c - 1;', '    SS16(0xFA0) = c - 1;'),
    (0x40034AB4, 'for (i = 1; i < n; i++) {', 'for (i = 0; i < n; i++) {'),
    (0x4003709C, '        if (e)\n', '        if (e > 1)\n'),
    (0x400347A0, 'SYS16(0x754) + 2);', 'SYS16(0x754) + 1);'),
    (0x40036958, 'SYS16(0x3DA52E) = 1;', 'SYS16(0x3DA52E) = 0;'),
    (0x40035780, '== 0)\n            r = 1;', '!= 1)\n            r = 1;'),
    (0x40035578, '    for (row = 0; row < 3; row++) {', '    for (row = 0; row < 2; row++) {'),
    (0x400366A0, '    if (SYS8(0x665))\n        state_restore_all();\n', '    state_restore_all();\n'),
    (0x400360F0, 'if (disk_free_bytes() < SYS32(0x774) * 2u) {', 'if (disk_free_bytes() <= SYS32(0x774) * 2u) {'),
    (0x40036EA4, 'c |= p[3] & 0xFF;', 'c |= p[3];'),
    (0x400367EC, '    else if (SYS16(8) - 12 < b)', '    else if (SYS16(8) - 11 < b)'),
    # batch 32: widgets, fonts, trace cursors
    (0x40019B8C, '    half_42e68be2_set((unsigned char)h);', '    half_42e68be2_set(h);'),
    (0x40019CF8, '        c = 400;', '        c = 399;'),
    (0x4001C200, '(unsigned short)(y + 24 - 2 * lv), 9, 2 * lv);', '(unsigned short)(y + 24 - 2 * lv), 9, lv);'),
    (0x4002DCAC, '                if (mx < v)', '                if (mx <= v && v != 7)'),
    (0x4002CDC4, '(code & 0xFF) - 0x4141)', '(code & 0xFF) - 0x4140)'),
    (0x4002D0D4, '(unsigned short)(y + 1 + j), s);', '(unsigned short)(y + j), s);'),
    (0x4002E6D8, '    for (r = 0; r < H16(FONT + 4) - 1; r++) {', '    for (r = 0; r < H16(FONT + 4); r++) {'),
    (0x4002E7B0, "    case '.': return 10;", "    case ',': return 10;"),
    (0x4002EA88, '    g = (unsigned char)(g ? g * 4 + 3 : 0);', '    g = (unsigned char)(g * 4 + 3);'),
    (0x40032D3C, '        else if (d >= 2)', '        else if (d >= 1)'),
    (0x40037944, '    else if (c < -v)', '    else if (c < v)'),
    (0x40037B48, '    } while (H32(4) >= v);', '    } while (H32(4) > v);'),
    # batch 33: drawing primitives, buttons, markers
    (0x4002EBC8, '    int end = y1 + 1;', '    int end = y1;'),
    (0x4002ECC4, '            o += 0x7FC;', '            o += 0x7FE;'),
    (0x4002EC4C, '    rect_fill(x, (unsigned short)(h + y + 0xFFFF), w, 1);', '    rect_fill(x, (unsigned short)(h + y), w, 1);'),
    (0x4002ED90, '            if (y >= prev)\n                vline_draw((unsigned short)x, (unsigned short)prev, (unsigned short)y);\n            else\n                vline_draw((unsigned short)x, (unsigned short)y, (unsigned short)prev);\n            x++;', '            vline_draw((unsigned short)x, (unsigned short)prev, (unsigned short)y);\n            x++;'),
    (0x4002D724, '        y = (unsigned short)y;\n', ''),
    (0x4002E878, '            cg = v * g0 / 255;', '            cg = v * g0 / 256;'),
    (0x40034134, '    if (align == 0 || align == 2) {', '    if (align == 0) {'),
    (0x4002F968, '    TBU(0x64C);\n', ''),
    (0x4002FB98, '    for (i = 0; i < SYS16(0xF9A); i++)', '    for (i = 1; i < SYS16(0xF9A); i++)'),
    (0x40030060, '        SYS8(0x801)++;', '        SYS8(0x800)++;'),
    (0x40032A58, '    SYS32(orng) = mx - mn;', '    SYS32(orng) = mn - mx;'),
    (0x40032A58, 'mx = 0, mn = 0xFFFFFF;', 'mx = 0, mn = 0xFFFF;'),
    # batch 34: view, zoom, MMU, FFT, boot steps
    (0x40042280, '    if (b < 1) {\n        hi = 0;', '    if (b < 1) {\n        hi = 1;'),
    (0x400423B8, '    if (hi > k)\n        hi = k;', '    if (hi < k)\n        hi = k;'),
    (0x400429E4, '        *p++ = 0xC12 | (s << 20) | cb;', '        *p++ = 0xC02 | (s << 20) | cb;'),
    (0x4004EC98, '    for (i = 0x82; i; i--) {', '    for (i = 0x81; i; i--) {'),
    (0x4004FAB0, '    } else if (r == -2) {\n        a = 1;', '    } else if (r == -3) {\n        a = 1;'),
    (0x40041A4C, '    off = (SYS32(0x454) - st * n) >> 1;', '    off = (SYS32(0x454) - st * n) >> 2;'),
    (0x40041E34, '    for (i = 0; i < SYS16(8) - 1u; i++, p++) {', '    for (i = 0; i < SYS16(8); i++, p++) {'),
    (0x40051C28, '    f_open(fil, path, 0xB);', '    f_open(fil, path, 0xA);'),
    (0x40051CF0, '        FS32(0) = 0x5A5555A5u;\n', ''),
    (0x40051E24, '    for (i = WS8(1); i >= 0; i--) {\n        if (WS8(2 + i) == 1) {', '    for (i = WS8(1); i > 0; i--) {\n        if (WS8(2 + i) == 1) {'),
    (0x40052098, '        if (*(volatile unsigned char *)(0x42E64520u + id * 14 + 2) == 0)', '        if (*(volatile unsigned char *)(0x42E64520u + id * 14 + 3) == 0)'),
    (0x400381E8, '    TD(0xACA0) = TD(0x20) - b1;', '    TD(0xACA0) = TD(0x28) - b1;'),
    # batch 35: button handlers, counters, levels
    (0x40055344, '    ui_window_close(0x19);', '    ui_window_close(0x1A);'),
    (0x40054EB8, '    list_a_current_load();\n', ''),
    (0x400555E8, '    if (TB(0x9EE) != 2)', '    if (TB(0x9EE) == 0)'),
    (0x40055720, '    } else if (TB(0x11A) == 2) {', '    } else if (TB(0x11A) == 3) {'),
    (0x40035540, '    SYS16(0xF9E) = 0;\n', ''),
    (0x40052F08, '        ui_window_open(0x18);', '        ui_window_open(0x14);'),
    (0x40055AD4, '        TB(0x174) = 0;\n        SYS8(0x7AC) = 4;', '        TB(0x174) = 0;\n        TB(0) = 0;\n        SYS8(0x7AC) = 4;'),
    (0x40055634, '            r1 = 1;', '            r1 = 0x4091F398u;'),
    (0x40055BD4, '    delay_ms(500);', '    delay_ms(50);'),
    (0x40056974, 'TX32(0x9C00 + 0x1A0)', 'TX32(0x9C00 + 0x1A4)'),
    (0x40054540, '        TB(0) = 1;', '        TB(0) = 0;'),
    (0x40063484, '        else if (v < 2 * m)', '        else if (v <= 2 * m + 1)'),
    (0x40063BC0, '        SYS8(0xBC0 + i) = 0;\n', ''),
    (0x40063C34, '    SYS8(0xBC0 + ch) = SYS8(0x67C + ch);', '    SYS8(0xBC0 + ch) = cur + 1;'),
    (0x400621EC, '        else if (v <= 0)\n            v = 1;', '        else if (v < 0)\n            v = 1;'),
    (0x40062284, '        else if (v < 0)\n            v = 0;', '        else if (v < 0)\n            v = 1;'),
    (0x40061580, '    if (n < (u32)(n << 1)) {', '    if (n > 1) {'),
    (0x40063798, '        if ((int)r < 0)', '        if ((int)r <= 0)'),
    (0x40062E14, '    MD(0x148) = MD(0x10) + d * 0.5;', '    MD(0x148) = MD(0x10) + d * 0.25;'),
    # batch 36: more button handlers, levels, FPGA params
    (0x40056214, '        CUR(4) = DG32(8) - DG32(0x30);', '        CUR(4) = DG32(8) + DG32(0x30);'),
    (0x40056724, '            DG64(8) = MK(0x60);', '            DG64(8) = (u32)MK(0x60);'),
    (0x400562F4, '    if (TT8(0xAD19) == 0 || TT8(0xAD1A) == 0)', '    if (TT8(0xAD19) == 0)'),
    (0x40056404, '        SYS32(0x5F0) = 100000000;', '        SYS32(0x5F0) = 10000000;'),
    (0x400524B4, '    ui_window_invalidate_from(3, v);', '    ui_window_invalidate_from(3, 0);'),
    (0x40055F2C, '        TT8(0x47) = v;\n', ''),
    (0x40055978, '    case 0: t = 0x680, p = 0x814; break;', '    case 0: t = 0x680, p = 0x815; break;'),
    (0x40056804, '            MK32(0xF4) = i;', '            MK32(0xF4) = i + 1;'),
    (0x40056080, '            numpad_open_mode1(0x201, 0xE9);', '            numpad_open_mode1(0x200, 0xE9);'),
    (0x400623F4, '        func_40061a70();\n', ''),
    (0x400634F0, '    fpga_param_updater(0x10, 2, w, (u32)(int)(short)SYS16(0x27E));', '    fpga_param_updater(0x10, 2, w, SYS16(0x27E));'),
    (0x40064C28, '    return c & b & a & 1;', '    return c & a & 1;'),
    # batch 37: output and menu buttons, numpad keys, tables
    (0x400537C4, '    for (i = 0; i < 9; i++) {', '    for (i = 0; i < 8; i++) {'),
    (0x40052100, '        if (SYS8(0x588) > 12) {', '        if (SYS8(0x588) >= 12) {'),
    (0x4006233C, '        v = sdv(sdv((int)((u32)sc * (u32)(int)*s++), d1), d2);', '        v = sdv((int)((u32)sc * (u32)(int)*s++), d1);'),
    (0x40064CF0, '    a &= func_400647f4(0x12, 5, 1);', '    a = func_400647f4(0x12, 5, 1);'),
    (0x4005FE94, '        if (NP8(0x17) < 10)', '        if (NP8(0x17) <= 10)'),
    (0x40061680, '                c = span / 2;', '                c = span / 2 + 1;'),
    (0x40063A50, '    p[7] = 0x343A3C40u;', '    p[7] = 0x343A3C41u;'),
    # batch 38: floating-point level functions
    (0x4006053C, '    p1 = pow(10.0, MD(0x88) / 20.0) * 1000.0;', '    p1 = pow(10.0, MD(0x88) / 10.0) * 1000.0;'),
    (0x40061DB0, '            q = -4095;', '            q = -4096;'),
    (0x40062B1C, '            a = (u32)((double)d * k1) + 0x11;', '            a = (u32)((double)d * k1) + 0x10;'),
    (0x40041D38, '        int v = (int)(float)(log10((double)*s++) * 45.0);', '        int v = (int)(log10((double)*s++) * 45.0 + 0.5);'),
    (0x40032B5C, '                *o1 = t * 2u;', '                *o1 = t;'),
    (0x40032B5C, '            if (diff >= (u32)(SYS16(8) * SYS32(0x558)) / 3u) {', '            if (diff >= (u32)(SYS16(8) * SYS32(0x558)) / 2u) {'),
    # batch 39: settings, list and trigger buttons
    (0x40052FC4, '        SYS8(s + 8) = 1;', '        SYS8(s + 8) = 2;'),
    (0x400531B0, '        if (SYS8(0x652))', '        if (SYS8(0x653))'),
    (0x40054F50, '            d = 1;', '            d = 2;'),
    (0x40054F50, '        SYS8(0x5DC6 + (short)SYS16(0xFA4)) = 1;', '        SYS8(0x5DC7 + (short)SYS16(0xFA4)) = 1;'),
    (0x40055114, '            nv = PS16(0x14) - P16(0xA);', '            nv = PS16(0x14) + P16(0xA);'),
    (0x40055114, '                if (tot >= c + 1) {', '                if (tot > c + 1) {'),
    (0x400525EC, '                SYS32(0x658) = 15 * i;', '                SYS32(0x658) = 16 * i;'),
    (0x40055798, '    SYS8(0x7C0) = i + 1;', '    SYS8(0x7C0) = i;'),
    (0x40053940, '                v = 3;', '                v = 2;'),
    (0x40056530, '            tgt = 0x42E68668u + 0x130, fl = 1, cb = CB, mode = 3;', '            tgt = 0x42E68668u + 0x130, fl = 1, cb = CB, mode = 4;'),
    # batch 40: list A / C windows, settings buttons, self-tests
    (0x40054C84, '                if (tot >= c + 1) {', '                if (tot >= c) {'),
    (0x40054C84, '            nv = P16(0xA) + PS16(0x14);', '            nv = P16(0xA) + PS16(0x14) + 1;'),
    (0x400553A0, '                        LC_REC = *(volatile unsigned short *)(0x40544E68u + 0xFA6 + 2 * c);', '                        LC_REC = *(volatile unsigned short *)(0x40544E68u + 0xFA6 + c);'),
    (0x400553A0, '            if (v) {', '            if (!v) {'),
    (0x400527C0, '        n = (int)((u32)(x - lo) * 100u);', '        n = (int)((u32)(x - lo) * 101u);'),
    (0x400527C0, '        else if (hi < x)', '        else if (hi < x - 1)'),
    (0x400527C0, '        ui_window_open(0x27);', '        ui_window_open(0x29);'),
    (0x40052280, '            if ((int)SYS8(0x41) < top)', '            if ((int)SYS8(0x41) <= top)'),
    (0x40052280, '        SYS32(0x120) = 0xFA;', '        SYS32(0x120) = 0xFB;'),
    (0x40055CC4, '                k = i - 9;', '                k = i - 8;'),
    (0x400504B8, '        a = (u32)(q2 + 0x18F) < 0x31Fu;', '        a = (u32)(q2 + 0x18F) <= 0x31Fu;'),
    (0x40050688, '            a &= q1 < -1499;', '            a &= q1 <= -1499;'),
    (0x40050688, '        st_step((u32)amp, 0x18, 6, i ? off : 0, 0);', '        st_step((u32)amp, 0x18, 6, off, 0);'),
    # batch 41: self-tests, touch test, dispatch, generator FPGA
    (0x40050BAC, '        for (j = 0x14; j < end; j++) {', '        for (j = 0x15; j < end; j++) {'),
    (0x40050BAC, '    p1 = b < (int)((float)a * 0.9f);', '    p1 = b <= (int)((float)a * 0.9f);'),
    (0x40050BAC, '    end = (int)SYS16(0x640) - 0x28;', '    end = (int)SYS16(0x640) - 0x27;'),
    (0x40050DDC, '    a = (long long)q2 * 5 < (long long)amp * 4;', '    a = (long long)q2 * 5 <= (long long)amp * 4;'),
    (0x40050DDC, '    SYS8(0x10A) = 2;', '    SYS8(0x10A) = 1;'),
    (0x400519E4, '            else if (IN(x, 0x1D9) && IN(TS16(6), 0x105))', '            else if (IN(x, 0x1D9) && IN(TS16(6), 0x106))'),
    (0x400519E4, '#define IN(v, lo) ((u32)((v) - (lo)) < 0x4Fu)', '#define IN(v, lo) ((u32)((v) - (lo)) < 0x50u)'),
    (0x40051E88, '    case 30:\n        touch_btn_01ec_release();', '    case 30:\n        touch_btn_019c_release();'),
    (0x4004ED18, '        if (c < 2500)\n            c = 2500;', '        if (c < 2400)\n            c = 2500;'),
    (0x4004ED18, '    if (c > f)', '    if (c >= f + 1 && f)'),
    (0x4004ED18, '    fpga_data_byte_write(((GB16(0x2EF6) - 2) >> 8) & 0xFF);', '    fpga_data_byte_write(((GB16(0x2EF6) - 1) >> 8) & 0xFF);'),
    # batch 42: self-tests, numpad parser, measurement limits
    (0x4004F89C, '    p1 = (double)e * 1.3 < (double)SYS32(0xB34) && (double)SYS32(0xB50) < (double)e * 0.7;', '    p1 = (double)e * 1.3 <= (double)SYS32(0xB34) && (double)SYS32(0xB50) < (double)e * 0.7;'),
    (0x4004F89C, '    SYS32(0x120) = e + 2 * h;\n    apply2();\n\n    e = SYS16(0xE);\n    p2', '    SYS32(0x120) = e + h;\n    apply2();\n\n    e = SYS16(0xE);\n    p2'),
    (0x4004F89C, '    p4 = (double)SYS32(0xB50) < (double)e * 0.7;', '    p4 = (double)SYS32(0xB34) < (double)e * 0.7;'),
    (0x4004F5E4, '        SYS16(0x44) = (int)(3000.0f + (float)s * 7.7777777f);', '        SYS16(0x44) = (int)(3000.0f + (float)s * 7.78f);'),
    (0x4004F5E4, '            for (b = 0; b <= 6; b++) {', '            for (b = 0; b < 6; b++) {'),
    (0x4004F5E4, '            for (i = 0; i < 880; i++) {\n                if', '            for (i = 1; i < 880; i++) {\n                if'),
    (0x4004F5E4, '            LANE8(k * 1000 + s) = *(volatile unsigned char *)(0x42DF76D4u + k * 0x2134);', '            LANE8(k * 1000 + s) = *(volatile unsigned char *)(0x42DF76D4u + k * 0x2130);'),
    (0x4005FC84, '            f = 10;', '            f = 100;'),
    (0x4005FC84, '    else if (m == 4 && unit == 0xF)', '    else if (m == 5 && unit == 0xF)'),
    (0x4005FC84, '            q = -0x7FFFFFFFLL;', '            q = -0x80000000LL;'),
    (0x4005FC84, '        if (ch - 0x30 >= 10)', '        if (ch - 0x30 > 10)'),
    (0x400617AC, '            M64(o) = 1000000000LL;', '            M64(o) = 999999999LL;'),
    (0x400617AC, '    else if (d < -60.0)', '    else if (d < -59.0)'),
    (0x400617AC, '#define LIM 0x1.0400000000001p+8', '#define LIM 259.5'),
    # batch 43: trace segment scaling, level columns, level search
    (0x4005F778, '        mul = 0x555;', '        mul = 0x556;'),
    (0x4005F778, '                y = mn;', '                y = mn + 1;'),
    (0x4005F778, '        mx = t + (int)(v / 3) - 5;', '        mx = t + (int)(v / 3) - 4;'),
    (0x4005F778, '            int a = (int)((off + (u32)(t >> 10)) * mul);', '            int a = (int)((off + (u32)(t >> 11)) * mul);'),
    (0x40061194, '        double k = 510.0 / (double)(u32)(h - MB32(0x174));', '        double k = 511.0 / (double)(u32)(h - MB32(0x174));'),
    (0x40061194, '                BUF[MB32(0xEC) * (u32)y + (u32)col] = COLOUR16;', '                BUF[MB32(0xEC) * (u32)y + (u32)col + 1] = COLOUR16;'),
    (0x40062C98, '                        p -= w;', '                        p -= w + 1;'),
    (0x40062C98, '        int top = TOP16(col), y = (int)h0, i = 0;', '        int top = TOP16(col), y = (int)h0, i = 1;'),
    (0x40060FE8, '            if ((int)l >= 7)', '            if ((int)l >= 6)'),
    (0x40060FE8, '    lo = (u32)((double)h * 0.2);', '    lo = (u32)((double)h * 0.25);'),
    (0x40060FE8, '    } while ((int)(MB32(oh) - MB32(0x184)) >= i);', '    } while ((int)(MB32(oh) - MB32(0x184)) > i);'),
    # batch 44: text and numbers
    (0x4002DD70, '        return (unsigned short)(c - 0x61u + 26);', '        return (unsigned short)(c - 0x61u + 27);'),
    (0x4002DD70, '    if (c - 0x30u <= 9)', '    if (c - 0x30u <= 8)'),
    (0x4002D164, '            w += sp + 14;', '            w += sp + 13;'),
    (0x4002D164, '        return (((u32)c << 6) & 0x7C0) | ((unsigned char)s[1] & 0x3F);', '        return (((u32)c << 6) & 0x7C0) | ((unsigned char)s[1] & 0x1F);'),
    (0x4002D164, '    return w - sp;', '    return w;'),
    (0x4002CE7C, '                x = SPACING + (H16(FONT + 8) + x);\n            }\n        } else if (t == 8) {', '                x = SPACING + (H16(FONT + 6) + x);\n            }\n        } else if (t == 8) {'),
    (0x4002CE7C, '    H16(0x42E68BE4u) = (unsigned short)(x - x0 + 1);', '    H16(0x42E68BE4u) = (unsigned short)(x - x0);'),
    (0x4002CE7C, '        *n = 3;', '        *n = 2;'),
    (0x4002D2D4, '    n = dec < top ? top : dec;', '    n = dec <= top ? top - 1 : dec;'),
    (0x4002D2D4, "        buf[p++] = neg ? '-' : '+';", "        buf[p++] = neg ? '-' : ' ';"),
    (0x4002D82C, '    d[15] = (int)(v / 1000000000000000LL);', '    d[15] = (int)(v / 100000000000000LL);'),
    (0x4002D82C, "    if (dec)\n        buf[p++] = '.';", "    if (dec > 1)\n        buf[p++] = '.';"),
    (0x4002EE78, '                if (a >= 0xF6) {', '                if (a >= 0xF5) {'),
    (0x4002EE78, '                if (a < 3)', '                if (a < 2)'),
    (0x4002EE78, '                            pg = pg * 4 + 3;', '                            pg = pg * 4 + 2;'),
    (0x4002EE78, '        idx += 1022;', '        idx += 1024;'),
    # batch 45: SD update check, flash read and RMW
    (0x40043620, '        cmd[0] = mode == 1 ? 0x3B : 0x03;', '        cmd[0] = mode == 2 ? 0x3B : 0x03;'),
    (0x40043620, '        cmd[0] = mode == 1 ? 0x3C : 0x13;', '        cmd[0] = mode == 1 ? 0x3C : 0x12;'),
    (0x40043620, '    if (mode > 0)\n        cmd[n++] = 0;', '    if (mode > 1)\n        cmd[n++] = 0;'),
    (0x40043620, '    if (READ_MODE > 0)\n        READ_MODE = 0;', '    if (READ_MODE > 1)\n        READ_MODE = 0;'),
    (0x40043620, '        cmd[1] = addr >> 24;', '        cmd[1] = addr >> 16;'),
    (0x40043874, '            if (SECTOR_BUF[off + i] != 0xFF)', '            if (SECTOR_BUF[off + i] != 0xFE)'),
    (0x40043874, '        chunk = (unsigned short)len;', '        chunk = (unsigned char)len;'),
    (0x40043874, '        chunk = len < 0x1001 ? (unsigned short)len : 0x1000;', '        chunk = len < 0x1001 ? (unsigned short)len : 0xFFF;'),
    (0x40043874, '            spi_flash_write_paged(dev, addr, (u32)buf, (unsigned short)chunk);', '            spi_flash_write_paged(dev, sector << 12, (u32)buf, (unsigned short)chunk);'),
    (0x4004F134, '        if (l == 0)\n            continue;', '        if (l == 1)\n            continue;'),
    (0x4004F134, '            info.fname[l + 4] == 0) {', '            1) {'),
    (0x4004F134, '    if (size < 1000) {', '    if (size < 999) {'),
    (0x4004F134, '    m = n - 36;', '    m = n - 37;'),
    (0x4004F134, '            if (c & (0x80u >> k))\n                sum[1 + k] += i;', '            if (c & (0x80u >> k))\n                sum[1 + k] += c;'),
    (0x4004F134, '        want[k] = be32(e + 4 * k);', '        want[k] = be32(e + 4 * (k ^ 1));'),
    (0x4004F134, '    persistence_write_read_modify_write(0, 0xA8000,', '    persistence_write_read_modify_write(0, 0xA9000,'),
    (0x4004F134, '    if (dir_open)\n        f_closedir(dir);', '    if (!dir_open)\n        f_closedir(dir);'),
    # batch 46: dialogs, colour buttons, XY grid, markers, cursors
    (0x400262F8, '    bx = cx - 0x78;', '    bx = cx - 0x77;'),
    (0x400262F8, '    text_shadow_draw(U16(H16(0x42E68BE4u) + x3), U16(ty), " ?", 1, 2, 0);', '    text_shadow_draw(U16(H16(0x42E68BE4u) + x3), U16(ty), " !", 1, 2, 0);'),
    (0x400271B0, '    x3 = U16(H16(0x42E68BE4u) + x2 + 8);', '    x3 = U16(H16(0x42E68BE4u) + x2 + 9);'),
    (0x400271B0, '    if (BTN[boff + 1]) {', '    if (BTN[boff]) {'),
    (0x40024E38, '    x2 = U16(H16(0x42E68BE4u) + x1 + (dx_in_x2 ? DX(118) : 0) + 8);', '    x2 = U16(H16(0x42E68BE4u) + x1 + 8);'),
    (0x40024E38, '    text_d6f8_shadow_draw(x2, U16(ty), H16(0x40544E68u + 0x88BE), 0, 0, 1, 2, 0);', '    text_d6f8_shadow_draw(x2, U16(ty), H16(0x40544E68u + 0x88BC), 0, 0, 1, 2, 0);'),
    (0x4002903C, '    iw = U16(w - 2);', '    iw = U16(w - 1);'),
    (0x4002903C, '    text_shadow_draw(U16(x2 + DX(b + 1) + xo), U16(y2 + DY(b + 1) + 0x39), TXS(b + 1), 1, 2, 0);', '    text_shadow_draw(U16(x2 + DX(b + 1) + xo), U16(y2 + DY(b + 1) + 0x38), TXS(b + 1), 1, 2, 0);'),
    (0x4002C32C, '    bw = U16(iw - 0x28);', '    bw = U16(iw - 0x27);'),
    (0x4002967C, '    draw_colour_set(COL(3));', '    draw_colour_set(COL(2));'),
    (0x40029920, '    rect_fill(x, y, 2, h);', '    rect_fill(x, y, 3, h);'),
    (0x40029EC0, '    touch_button_set_rect(U16(bx), U16(y2 + 0x64 + bd), bw, 0x3C, BTN + boff);', '    touch_button_set_rect(U16(bx), U16(y2 + 0x64), bw, 0x3C, BTN + boff);'),
    (0x40028084, '        if (BTN[0x9F9 + 10 * i] == 1) {', '        if (BTN[0x9F9 + 10 * i] != 0) {'),
    (0x40028084, '        if (i >= 2) {', '        if (i >= 1) {'),
    (0x4002C5F8, '            func_4002ecc4_span(U16((int)(base + (float)i * 12.6f)), U16(cx - 2), U16(cx + 2));', '            func_4002ecc4_span(U16((int)(base + (float)i * 12.5f)), U16(cx - 2), U16(cx + 2));'),
    (0x4002C5F8, '        for (p = left; p <= right; p += step)', '        for (p = left; p < right; p += step)'),
    (0x4002C5F8, '    int left = (int)H16(SYS + 4) + ((int)(H16(SYS + 8) - side) / 2);', '    int left = (int)H16(SYS + 4) + (int)((H16(SYS + 8) - side) >> 1);'),
    (0x40024C60, '        lo = (bot - 8 > y) ? y : bot - 8;', '        lo = (bot - 8 >= y) ? bot - 8 : y;'),
    (0x40024C60, '        func_4002ecc4_span(U16(lo + i), U16(x), U16(x + 7 - i));', '        func_4002ecc4_span(U16(lo + i), U16(x), U16(x + 8 - i));'),
    (0x40024C60, '    text_draw(U16(x + 0xFFFF), U16(k - 0xB), "1");', '    text_draw(U16(x), U16(k - 0xB), "1");'),
    (0x40029BC0, '    rect_fill(0x190, 0x1E1, 0xC8, 3);', '    rect_fill(0x190, 0x1E1, 0xC8, 4);'),
    (0x40029BC0, '    by = y + h - 0x32;', '    by = y + h - 0x30;'),
    (0x400292E4, '        if (BTN[0x157 + 10 * i] || *(volatile unsigned char *)(0x40544E68u + 0x7AC) == i + 1) {', '        if (BTN[0x157 + 10 * i] || *(volatile unsigned char *)(0x40544E68u + 0x7AC) == i) {'),
    (0x400292E4, '    for (i = 0, row = y + 0x14; i < 4; i++, row += 0x3C) {', '    for (i = 0, row = y + 0x14; i < 4; i++, row += 0x3B) {'),
    (0x40028364, '    n = (int)(bottom - top + 1) / 4;', '    n = (int)(bottom - top) / 4;'),
    (0x40028364, '    half = (int)(c2 - c1 - 0x3C) / 2;', '    half = (int)(c2 - c1 - 0x3C) >> 1;'),
    (0x40028364, '    marker(c2 + H16(SYS + 4), top, "2", U16(r5 - 0xB), U16(r5 + 0xFFFE), light, 2);', '    marker(c2 + H16(SYS + 4), top, "2", U16(r5 - 0xB), U16(r5 + 0xFFFE), light, 1);'),
    (0x40028364, '    if (B8(CUR + 4) == 0) {', '    if (B8(CUR + 5) == 0) {'),
]


def main():
    # A rename in a later batch can change the text a mutant expects. Check them all first, so a
    # stale mutant stops the run with a list instead of an assertion half-way through.
    stale = [(o, old) for o, old, _ in MUTANTS
             if old not in next(ROOT.glob(f"nonmatching/**/func_{o:08x}.c")).read_text()]
    for o, old in stale:
        print(f"STALE   0x{o:08x}: {old!r} is no longer in the source")
    if stale:
        sys.exit(2)
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
            suite = next(s for s in (fatfs.TESTS, fatfs_fs.TESTS, rtlib.TESTS, drivers.TESTS, usb.TESTS, platform.TESTS, fpga.TESTS, sweep.TESTS) if orig in s)
            _, fails, _ = run_one(orig, suite[orig], 40 if suite is fatfs_fs.TESTS else (80 if suite in (drivers.TESTS, usb.TESTS, platform.TESTS, fpga.TESTS, sweep.TESTS) else 400), 7)
            ok = bool(fails)
            caught += ok
            print(f"{'caught ' if ok else 'MISSED '} 0x{orig:08x}: {old!r} -> {new!r}")
    for orig, _, _ in MUTANTS:                       # restore genuine builds
        nmbuild.build(next(ROOT.glob(f"nonmatching/**/func_{orig:08x}.c")), syms)
    print(f"{caught}/{len(MUTANTS)} mutants detected")
    sys.exit(0 if caught == len(MUTANTS) else 1)


if __name__ == "__main__":
    main()
