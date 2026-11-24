/*
 * bode_autoscale -- stock 0x4003963C (672 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: fits the n = word +0x5C points of the last sweep into 0.8 x the plot height H (u16 +6). Gain (doubles +0x3EE8[i], dB): scale = 0.8H x 10000 / (max - min of the values x 10000), at most 25 when it would be 26 or more; pixel row = (int)(v x scale) + H/2 - c with c = (min + max) x scale / 20000, kept in 3..H, into u16 +0x8D20[i]; the axis range goes to +0x10 (top) / +0x18 (bottom). Phase (s16 +0x7D68[i], degrees): the same with a spread of at least 50000 and a scale of at most 3 (min / max start at 0x3FFFFFFF / 0xC0000001), rows into +0x9CC0[i], range into +0x20 / +0x28. With n = 0 the sums wrap as in the stock code.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "bode.h"
#include "shared/access.h"
#include "state/bode_block.h"

#define U16(v) ((unsigned short)(v))

static int clamp_row(int p)
{
    int h;
    if (p <= 2)
        return 3;
    h = BODES(plot_h);
    return h > p ? p : h;
}

void bode_autoscale(void)
{
    u32 H, half, n, k;
    int hh, mx, mn, v, c, lim;
    double base, sc, hd;
    if (g_touch->serviced)
        return;
    H = BODES(plot_h);
    half = H >> 1;
    hh = (int)((double)H * 0.8);
    n = BODES(freq_count);
    mx = (int)0xC0000001u;
    mn = 0x3FFFFFFF;
    for (k = 0; k < n; k++) {
        v = (int)(BODES(gain_db[k]) * 10000.0);
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
    }
    base = (double)hh * 10000.0;
    sc = base / (double)(int)((u32)mx - (u32)mn);
    if ((int)sc >= 26)
        sc = 25.0;
    c = (int)((double)(int)((u32)mn + (u32)mx) * sc / 20000.0);
    for (k = 0; k < n; k++)
        BODES(gain_rows[k]) = (unsigned short)clamp_row((int)((u32)(int)(BODES(gain_db[k]) * sc) + (half - (u32)c)));
    BODES(gain_bottom) = 0.0;
    hd = (double)(int)half;
    {
        double Hd = (double)BODES(plot_h), cd = (double)c, lo = (cd - hd) / sc, hi = (Hd - hd + cd) / sc;
        BODES(gain_bottom) = lo;
        BODES(gain_top) = hi;
    }
    mx = (int)0xC0000001u;
    mn = 0x3FFFFFFF;
    for (k = 0; k < n; k++) {
        v = (short)BODES(phase_deg[k]) * 10000;
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
    }
    lim = (int)((u32)mx - (u32)mn);
    if (lim < 50000)
        lim = 50000;
    sc = base / (double)lim;
    if ((int)sc > 3)
        sc = 3.0;
    c = (int)((double)(int)((u32)mn + (u32)mx) * sc / 20000.0);
    for (k = 0; k < n; k++)
        BODES(phase_rows[k]) = (unsigned short)clamp_row((int)((u32)(int)((double)(short)BODES(phase_deg[k]) * sc) + (half - (u32)c)));
    BODES(phase_bottom) = 0.0;
    {
        double Hd = (double)BODES(plot_h), cd = (double)c, lo = (cd - hd) / sc, hi = (Hd - hd + cd) / sc;
        BODES(phase_bottom) = lo;
        BODES(phase_top) = hi;
    }
}
