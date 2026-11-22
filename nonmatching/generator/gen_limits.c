/*
 * gen_limits -- stock 0x4004CF34 (352 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clamps the GEN settings (G+0x2EFC): amplitude to [0, max], offset to +-limit and offset +- amplitude/2 into the output range, waveform index, duty to [1, max], frequency to the waveform's maximum (50/10/5/3 MHz; above index 14 the stock code uses the r2/r3 it was called with). Skipped while the touch service is busy.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"

void gen_limits(u32 r0, u32 r1, u32 lo, u32 hi)
{
    volatile int *g = (volatile int *)(GEN_G + 0x2EFC);
    int off, amp, lim, half, nw, d, fr;
    if (g_touch->serviced != 0)
        return;
    off = g[2];
    amp = g[1];
    if (g[7] < amp) g[1] = g[7];
    else if (amp < 0) g[1] = 0;
    lim = g[8];
    if (lim < off) off = lim;
    else if (off < -lim) off = -lim;
    amp = g[1];
    half = (amp + (int)((u32)amp >> 31)) >> 1;
    if (g[10] < off + half) off = g[10] - half;
    else if (off - half < g[11]) off = g[11] + half;
    nw = g[5];
    if (nw < (int)GEN_WAVE) GEN_WAVE = nw;
    d = g[3];
    if (g[9] < d) g[3] = g[9];
    else if (d <= 0) g[3] = 1;
    switch (GEN_WAVE) {
    case 0: case 9: lo = 50000000; hi = 0; break;
    case 1: case 6: case 7: lo = 10000000; hi = 0; break;
    case 2: case 3: case 4: case 5: case 11: case 12: case 13: lo = 5000000; hi = 0; break;
    case 8: case 10: case 14: lo = 3000000; hi = 0; break;
    default: break;
    }
    fr = g[0];
    if ((long long)(((unsigned long long)hi << 32) | lo) < (long long)fr) g[0] = lo;
    else if (fr < 0) g[0] = 0;
    g[2] = off;
}
