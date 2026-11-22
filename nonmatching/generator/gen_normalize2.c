/*
 * gen_normalize2 -- stock 0x4004D618 (272 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Second table: copy from G+0x1F56, invert, scale to span 2 x amplitude (G+0x2F30) and shift by G+0x2F48 - amplitude, in double precision (truncated).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "util.h"

void gen_normalize2(void)
{
    int mx = 0, mn = 0x7FFFFFFF;
    u32 n, i;
    double sc;
    copy_u16(GEN_BUF2, (const unsigned short *)(GEN_G + 0x1F56), G16(0x2EF8));
    n = G16(0x2EF8);
    for (i = 0; i < n; i++) {
        int v = GEN_BUF2[i];
        if (v >= mx) mx = v;
        if (v <= mn) mn = v;
    }
    if (n != 0) {
        i = 0;
        do {
            GEN_BUF2[i] = mx - GEN_BUF2[i];
            i++;
        } while ((int)i < (int)G16(0x2EF8));
    }
    sc = (double)G32(0x2F30) * 2.0;
    sc = sc / (double)(mx - mn);
    if (G16(0x2EF8) != 0) {
        i = 0;
        do {
            double t = (double)G32(0x2F48);
            t = t + (double)GEN_BUF2[i] * sc;
            t = t - (double)GS32(0x2F30);
            GEN_BUF2[i] = (int)t;
            i++;
        } while ((int)i < (int)G16(0x2EF8));
    }
}
