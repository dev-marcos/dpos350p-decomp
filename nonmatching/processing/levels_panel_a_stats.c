/*
 * levels_panel_a_stats -- stock 0x40062284 (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When byte +1 of block 0x42E68668 is set: for n = word +0xDC samples, v = sample * scale (+0xE0) / word +0xC0, clamped to 0 .. scale, stores scale - v as u16 at 0x42E60008 and keeps the mean (+0x178), max (+0x17C) and min (+0x180).
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

void levels_panel_a_stats(void)
{
    int n, sc, dv, mx = -32767, mn = 32767, i, v;
    u32 sum = 0;
    volatile short *s;
    volatile unsigned short *d = (volatile unsigned short *)0x42E60008u;
    if (LEVELS(panel_a_on) == 0)
        return;
    sc = MB32(0xE0);
    n = MB32(0xDC);
    s = SRC;
    dv = MB32(0xC0);
    for (i = 0; i < n; i++) {
        v = sdv((int)((u32)sc * (u32)(int)*s++), dv);
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
    LEVELS(f_178) = sdv((int)sum, n);
    LEVELS(f_17c) = mx;
    LEVELS(f_180) = mn;
}
