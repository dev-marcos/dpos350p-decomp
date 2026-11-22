/*
 * measure_values_clamp -- stock 0x400617AC (672 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Numpad callback of the measurement window (skipped while byte 0x42E68BC7 is set): the four s64 values +0x120..+0x138 of block 0x42E68668 are limited to 0..1e9, the window recalculated (window_64_recalc) and the four limited again; the doubles +0x148 to -60..260 and +0x140 to 1..260 (a value is cut to 260 from 260 + 1 ulp up).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void window_64_recalc(void);
#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))
#define MD(o) (*(volatile double *)(0x42E68668u + (o)))
#define LIM 0x1.0400000000001p+8

static void clamp4(void)
{
    u32 o;
    for (o = 0x120; o <= 0x138; o += 8) {
        long long v = M64(o);
        if (v < 0)
            M64(o) = 0;
        else if (v > 1000000000LL)
            M64(o) = 1000000000LL;
    }
}

void measure_values_clamp(void)
{
    double d;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    clamp4();
    window_64_recalc();
    clamp4();
    d = MD(0x148);
    if (d >= LIM)
        MD(0x148) = 260.0;
    else if (d < -60.0)
        MD(0x148) = -60.0;
    d = MD(0x140);
    if (d >= LIM)
        MD(0x140) = 260.0;
    else if (d < 1.0)
        MD(0x140) = 1.0;
}
