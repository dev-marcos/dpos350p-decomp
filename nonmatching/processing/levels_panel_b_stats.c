/*
 * levels_panel_b_stats -- stock 0x4006233C (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When byte +2 of block 0x42E68668 is set (stock: falls into 0x40062354 with r3 = the block, ip = word +0xF0): for n = word +0xEC samples, v = sample * scale (+0xF0) / word +0xC0 / word +0x188 (sdiv), clamped to 0 .. scale, stores scale - v as u16 at 0x42E61000 and keeps the mean (+0x16C), max (+0x170) and min (+0x174).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "state/levels_block.h"

#define MB32(o) (*(volatile int *)(0x42E68668u + (o)))
#define SRC (*(volatile short *volatile *)0x42E68CB0u)

static int sdv(int a, int b)
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int)(0u - (u32)a);
    return a / b;
}

void levels_panel_b_stats(void)
{
    int n, sc, d1, d2, mx = -32767, mn = 32767, i, v;
    u32 sum = 0;
    volatile short *s;
    volatile unsigned short *d = (volatile unsigned short *)0x42E61000u;
    if (LEVELS(panel_b_on) == 0)
        return;
    sc = MB32(0xF0);
    n = MB32(0xEC);
    s = SRC;
    d2 = LEVELS(f_188);
    d1 = MB32(0xC0);
    for (i = 0; i < n; i++) {
        v = sdv(sdv((int)((u32)sc * (u32)(int)*s++), d1), d2);
        if (sc < v)
            v = sc;
        else if (v < 0)
            v = 0;
        v = sc - v;
        *d++ = v;
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
        sum += v;
    }
    LEVELS(f_16c) = sdv((int)sum, n);
    LEVELS(f_170) = mx;
    LEVELS(f_174) = mn;
}
