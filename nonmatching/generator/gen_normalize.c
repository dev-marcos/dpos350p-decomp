/*
 * gen_normalize -- stock 0x4004D390 (268 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies the loaded samples (G+0x1F56) to the table, inverts them (max - v) and scales them to span 2 x (G+0xFB0 / 2) in double precision (truncated).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "util.h"

void gen_normalize(void)
{
    u32 half = G16(0xFB0) >> 1;
    int mx = 0, mn = 0x7FFFFFFF;
    u32 n, i;
    double sc, hd;
    copy_u16(GEN_BUF, (const unsigned short *)(GEN_G + 0x1F56), G16(0x2EF8));
    n = G16(0x2EF8);
    for (i = 0; i < n; i++) {
        int v = GEN_BUF[i];
        if (v >= mx) mx = v;
        if (v <= mn) mn = v;
    }
    if (n != 0) {
        i = 0;
        do {
            GEN_BUF[i] = mx - GEN_BUF[i];
            i++;
        } while ((int)i < (int)G16(0x2EF8));
    }
    sc = (double)(int)half * 2.0;
    sc = sc / (double)(mx - mn);
    hd = (double)(int)half;
    if (G16(0x2EF8) != 0) {
        i = 0;
        do {
            double t = hd;
            t = t + (double)GEN_BUF[i] * sc;
            t = t - hd;
            GEN_BUF[i] = (int)t;
            i++;
        } while ((int)i < (int)G16(0x2EF8));
    }
}
