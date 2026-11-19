/*
 * trace_intensity_accumulate_window -- stock 0x4005E838 (1412 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Capture-window version of trace_intensity_accumulate_lines for trace tr: only when SYS+0x6FC >= SYS+0x708; the columns are c0 = SYS+0x700 .. c0 + u16 SYS+0x6F8 - 1 and only their flags (from 0x426BFF94 + 2 c0, u16 SYS+0x6F8 entries) are cleared. Same stamping, covered-cell increment 5 x segments + 20 and always four smoothing passes; the divisor SYS+0x7B0 becomes 45 x W32 SYS+0x5F8 / 50.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))

#define HITS(tr) (0x400954A4u + (tr) * 0x12BE70u)
#define SMP(tr) (0x413F92E8u + (tr) * 0x927C0u)
#define COLB 0x426BFF94u   /* per column: 600 u16 "row covered" flags (0x4B0 bytes) */
#define MAXL 0x42E63528u   /* per column: highest covered row + 1 (u16) */
#define MINL 0x42E63D24u   /* per column: lowest covered row - 1 (u16) */

u32 mode_not_0_or_1(u32 v);
void fill_u16(unsigned short *d, unsigned short v, u32 n);

#define ADD16(a, k) (H16(a) = (unsigned short)(H16(a) + (k)))

/* register shift as the core does it: the low byte of n, 32 and more give 0 */
static u32 shl(u32 v, int n)
{
    u32 k = (u32)n & 0xFF;
    return k >= 32 ? 0 : v << k;
}

/* Stamps the K x nseg points of each column x0 .. lim - 1 into the hit counts (row split over
   three cells by the fraction of L) and marks the covered rows; returns the sum of (max - min). */
static int cols_stamp(u32 tr, int x0, int lim, u32 h, int ymax, u32 nseg, u32 k, u32 lh)
{
    u32 l = W32(SYS + 0x1C0), sh = W32(SYS + 0x1BC), base = HITS(tr), coloff, colbuf, pt, s, v, frac, cell;
    int x, mx, mn, hi, lo, row, r, n, tot = 0;
    volatile unsigned short *ip;
    if (x0 >= lim)
        return 0;
    coloff = h * (u32)x0;
    colbuf = COLB + 0x4B0 * (u32)x0;
    for (x = x0; x < lim; x++) {
        mx = 0;
        mn = 0xFFFF;
        if ((int)nseg > 0) {
            pt = (u32)x * (k * nseg);
            for (s = nseg; s; s--) {
                hi = 0;
                lo = 0xFFFF;
                if ((int)k > 0) {
                    ip = (volatile unsigned short *)(SMP(tr) + pt * 2);
                    for (n = (int)k; n; n--) {
                        v = lh - *ip++;
                        row = (int)v >> sh;
                        frac = (l - 1) & v;
                        if (ymax < row)
                            row = ymax;
                        if (row >= hi)
                            hi = row;
                        if (row <= lo)
                            lo = row;
                        cell = base + 2 * ((u32)row + coloff);
                        ADD16(cell - 2, l - frac);
                        ADD16(cell, 0x40);
                        ADD16(cell + 2, frac);
                    }
                }
                for (r = lo - 1; r <= hi + 1; r++)
                    H16(colbuf + 2 * (u32)r) = 1;
                if (hi + 1 >= mx)
                    mx = hi + 1;
                if (lo - 1 <= mn)
                    mn = lo - 1;
                pt += k;
            }
        }
        H16(MAXL + 2 * (u32)x) = (unsigned short)mx;
        H16(MINL + 2 * (u32)x) = (unsigned short)mn;
        tot += mx - mn;
        colbuf += 0x4B0;
        coloff += h;
    }
    return tot;
}

/* Adds add to every covered cell of column x. */
static void col_fill(u32 base, u32 colbuf, u32 coloff, int x, u32 add)
{
    int r = (int)H16(MINL + 2 * (u32)x) - 1;
    u32 cell;
    if ((int)H16(MAXL + 2 * (u32)x) + 1 < r)
        return;
    cell = (u32)r + coloff;
    do {
        short v = (short)H16(colbuf + 2 * (u32)r);
        r++;
        if (v)
            ADD16(base + 2 * cell, add);
        cell++;
    } while ((int)H16(MAXL + 2 * (u32)x) + 1 >= r);
}

/* One smoothing pass of the covered run lo .. hi of a column: moving sum over 2w cells
   (w = 2^(s-1)) written back w cells behind, as sum >> s; uncovered cells restart it. */
static void col_smooth(u32 base, u32 colbuf, u32 coloff, int lo, int hi, int s)
{
    unsigned short ring[602];
    int w = (int)shl(1, s - 1), c = 0, sum = 0, j = lo;
    u32 cell, c16, hv;
    if (hi < j)
        return;
    cell = (u32)j + coloff;
    do {
        if ((short)H16(colbuf + 2 * (u32)j) == 0) {
            sum = 0;
            c = 0;
        } else {
            c16 = (u32)c & 0xFFFF;
            hv = H16(base + 2 * cell);
            sum += (int)hv;
            ring[c16] = (unsigned short)hv;
            if ((int)c16 >= w * 2) {
                H16(base + 2 * (cell - (u32)w)) = (unsigned short)(sum >> s);
                c16 -= 2 * (u32)w;
                sum -= ring[c16];
            }
            c++;
        }
        j++;
        cell++;
    } while (hi >= j);
}

void trace_intensity_accumulate_window(u32 tr)
{
    u32 c0, h, lh, m, nseg, np, on, k, coloff, colbuf, base, add, hh;
    int lim, ymax, x, s;
    if (W32(SYS + 0x6FC) < W32(SYS + 0x708))
        return;
    c0 = W32(SYS + 0x700);
    lim = (int)(c0 + H16(SYS + 0x6F8));
    h = H16(SYS + 0xA);
    lh = W32(SYS + 0x1C0) * h;
    m = mode_not_0_or_1(tr);
    nseg = H16(SYS + 0x5F4);
    fill_u16((unsigned short *)(COLB + 2 * W32(SYS + 0x700)), 0, H16(SYS + 0x6F8));
    h = H16(SYS + 0xA);
    if (B8(SYS + 0x2AA) && m == 0)
        ymax = (int)(h >> 1) - 3;
    else
        ymax = (int)h - 3;
    if (m) {
        on = H16(SYS + 0x1A);
        np = W32(SYS + 0x610);
    } else {
        on = H16(SYS + 0x14);
        np = W32(SYS + 0x5F8);
    }
    if (on == 0)
        return;
    k = 1 + 2 * np;
    cols_stamp(tr, (int)c0, lim, h, ymax, nseg, k, lh);
    W32(SYS + 0x7B0) = 0;
    add = 5 * nseg + 20;
    if ((int)c0 < lim) {
        hh = h;
        coloff = hh * c0;
        colbuf = COLB + 0x4B0 * c0;
        base = HITS(tr);
        for (x = (int)c0; x < lim; x++) {
            col_fill(base, colbuf, coloff, x, add);
            for (s = 4; s > 0; s--)
                col_smooth(base, colbuf, coloff, (int)H16(MINL + 2 * (u32)x) - 1, (int)H16(MAXL + 2 * (u32)x) + 1, s);
            colbuf += 0x4B0;
            coloff += hh;
        }
    }
    W32(SYS + 0x7B0) = 45 * W32(SYS + 0x5F8) / 50;
}
