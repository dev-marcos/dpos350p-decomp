/*
 * fft_levels_db -- stock 0x40061DB0 (300 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: for the n = word +0xF8 FFT magnitudes (floats at 0x42D364B4, divided by n): 20 * log10(m * 6.916 / g * f / 1000 * 80 / g) (g = u16 SYS+0x20, f = u64 SYS+0x50), plus offset +0xD0 * step +0x140, minus top +0x148, times scale +0xCC, over step; as s64, clamped to -4095 .. 4095 and stored as s16 through the pointer 0x42E68CB0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define MI(o) (*(volatile int *)(0x42E68668u + (o)))
#define MD(o) (*(volatile double *)(0x42E68668u + (o)))
#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))

double log10(double x);

void fft_levels_db(void)
{
    int n, i;
    volatile float *m = (volatile float *)0x42D364B4u;
    volatile short *dst;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    n = MI(0xF8);
    if (n < 1)
        return;
    dst = *(volatile short *volatile *)0x42E68CB0u;
    for (i = 0; i < n; i++) {
        float v = m[i] / (float)n;
        double g = (double)SYS16(0x20);
        double fr = (double)*(volatile unsigned long long *)(0x40544E68u + 0x50);
        double t = (double)v * 6.916 / g * fr / 1000.0 * 80.0 / g;
        double r = log10(t) * 20.0 + (double)MI(0xD0) * MD(0x140);
        long long q = (long long)((r - MD(0x148)) * (double)MI(0xCC) / MD(0x140));
        if (q < -4095)
            q = -4095;
        else if (q >= 0x1000)
            q = 0xFFF;
        *dst++ = (short)q;
    }
}
