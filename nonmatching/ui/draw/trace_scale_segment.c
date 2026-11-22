/*
 * trace_scale_segment -- stock 0x4005F778 (352 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Scales trace tr's raw samples start..start+len (u16, buffer 0x42AC7CF4 + sel * 0x62250 with sel = mode_not_0_or_2(tr)) into screen samples (u16, 0x411AF3E8 + tr * 0x927C0): y = base + ((off + ((x - off - pos) * gain >> 10)) * mul >> 12), clamped to min..max. k = SYS+0x1C0; gain / pos / offset are SYS+0xE4 / +0xE8 / +0x60 (sel 0) or +0x1A4 / +0x1A8 / +0x120; off = k * offset, pos = k * pos. Full screen (SYS+0x2AA = 0): mul 0x1000, range 4..height-5 (height = SYS+0xA); split screen: the lower third (mul 0x555, base 2h/3, mode_not_0_or_1(tr) = 0) or the upper two thirds (mul 0xAAB). min, max and base are scaled by k.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "ui/draw.h"

void trace_scale_segment(u32 tr, u32 start, u32 len)
{
    int end = (int)(len + start + 1);
    u32 sel = mode_not_0_or_2(tr);
    u32 k = SYS32(0x1C0), gain, pos, off, mul;
    int mx, mn, base;
    if (sel == 0) {
        gain = SYS32(0xE4);
        pos = SYS32(0xE8);
        off = SYS32(0x60);
    } else {
        gain = SYS32(0x1A4);
        pos = SYS32(0x1A8);
        off = SYS32(0x120);
    }
    off = k * off;
    if (SYS8(0x2AA) == 0) {
        mx = (int)SYS16(0xA) - 5;
        mul = 0x1000;
        base = 0;
        mn = 4;
    } else if (mode_not_0_or_1(tr) == 0) {
        u32 v = SYS16(0xA);
        int t = (int)(2 * v) / 3;
        mx = t + (int)(v / 3) - 5;
        mn = t + 4;
        mul = 0x555;
        base = t;
    } else {
        u32 v = SYS16(0xA);
        mx = (int)(2 * v) / 3 - 5;
        mul = 0xAAB;
        base = 0;
        mn = 4;
    }
    k = SYS32(0x1C0);
    mx = (int)((u32)mx * k);
    mn = (int)((u32)mn * k);
    base = (int)((u32)base * k);
    pos = k * pos;
    if ((int)start < end) {
        volatile unsigned short *s = (volatile unsigned short *)(0x42AC7CF4u + sel * 0x62250u) + start;
        volatile unsigned short *d = (volatile unsigned short *)(0x411AF3E8u + tr * 0x927C0u) + start;
        int n = end - (int)start;
        do {
            int t = (int)(((u32)*s++ - off - pos) * gain);
            int a = (int)((off + (u32)(t >> 10)) * mul);
            int y = (int)((u32)base + (u32)(a >> 12));
            if (mx <= y)
                y = mx;
            if (mn >= y)
                y = mn;
            *d++ = (unsigned short)y;
        } while (--n);
    }
}
