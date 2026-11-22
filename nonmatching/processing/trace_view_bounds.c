/*
 * trace_view_bounds -- stock 0x4003BB6C (300 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For modes other than 0 and 1: view_bounds_update. Otherwise the visible sample range of the screen: lo = W(SYS+0x608) + u16 SYS+0xC clamped to 0..width (u16 SYS+8), hi = W(SYS+0x60C) + u16 +0xC - 1 clamped to 0..width - 1 (0 when the sum is below 1), hi raised to lo; when the record (SYS+0x5D4) is shorter than SYS+0x5F8, the range is also cut to the centred part width x SYS+0x5D4 / SYS+0x5F8 of the screen. Results: u16 SYS+0x10 = lo, +0x12 = hi, +0x14 = hi - lo.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define I32(a) (*(volatile int *)(a))
#define S64(a) (*(volatile long long *)(a))
#define P(o) (SYS + 0x5F4 + (o))
#define TRBUF(ch) (0x411AF3E8u + (ch) * 0x927C0u)

void trace_view_bounds(u32 tr)
{
    u32 c, w, lo, hi, top, a, b;
    long long t;
    int q, half, qh, l1, h1;
    if (mode_not_0_or_1(tr)) {
        view_bounds_update();
        return;
    }
    c = H16(SYS + 0xC);
    t = (long long)I32(SYS + 0x608) + c;
    if (t < 0)
        lo = 0;
    else {
        lo = H16(SYS + 8);
        if (t < (long long)lo)
            lo = (u32)t;
    }
    H16(SYS + 0x10) = (unsigned short)lo;
    t = (long long)I32(SYS + 0x60C) + c;
    if (t < 1)
        hi = 0;
    else {
        w = H16(SYS + 8);
        if (t < (long long)w - 1)
            hi = (u32)t + 0xFFFF;
        else
            hi = w + 0xFFFF;
    }
    a = W32(SYS + 0x5F8);
    b = W32(SYS + 0x5D4);
    top = (unsigned short)lo;
    if (top < (unsigned short)hi)
        top = (unsigned short)hi;
    hi = top - lo;
    if (b < a) {
        w = H16(SYS + 8);
        q = (int)((long long)((unsigned long long)w * b) / (long long)a);
        half = (int)(w / 2);
        qh = q / 2;
        l1 = half - qh + 1;
        h1 = qh + half - 1;
        if (l1 > (int)(unsigned short)lo)
            lo = (u32)l1;
        if (h1 < (int)top)
            top = (u32)h1;
        hi = top - lo;
    }
    H16(SYS + 0x14) = (unsigned short)hi;
    H16(SYS + 0x12) = (unsigned short)top;
    H16(SYS + 0x10) = (unsigned short)lo;
}
