/*
 * fft_db_traces_scale -- stock 0x40041D38 (252 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For each enabled channel (SYS+0x3E/+0x42, SYS+0x102/+0x106) and each of the n = SYS+0x7F8 FFT values (floats at 0x42E4BEB4 / 0x42E4FEB4): v = (int)(float)(45 * log10(x)), clamped to 0 .. h (h = half the height SYS+0xA), stored as h - v (u16) at 0x42E5B010 / 0x42E5C010 for fft_traces_draw.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

double log10(double x);

static void scale(volatile float *s, volatile unsigned short *d, u32 n, int h)
{
    for (; n; n--) {
        int v = (int)(float)(log10((double)*s++) * 45.0);
        if (h < v)
            v = h;
        else if (v < 0)
            v = 0;
        *d++ = h - v;
    }
}

void fft_db_traces_scale(void)
{
    int h = SYS16(0xA) >> 1;
    if (SYS8(0x3E) && SYS8(0x42) && SYS32(0x7F8))
        scale((volatile float *)0x42E4BEB4u, (volatile unsigned short *)0x42E5B010u, SYS32(0x7F8), h);
    if (SYS8(0x102) && SYS8(0x106) && SYS32(0x7F8))
        scale((volatile float *)0x42E4FEB4u, (volatile unsigned short *)0x42E5C010u, SYS32(0x7F8), h);
}
