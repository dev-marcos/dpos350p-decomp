/*
 * trace_intensity_accumulate_lines -- stock 0x4005E12C (1764 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Line-mode hit accumulation of trace tr (u8): clears the column flags 0x426BFF94 (0x95B50 u16), then for the columns x0 .. x1 - 2 of the plot (u16 SYS+0x10 / +0x12, points W32 SYS+0x5F8, gate u16 SYS+0x14; the second window SYS+0x16 / +0x18 / +0x610 / +0x1A when mode_not_0_or_1(tr)) stamps each of the (2 points + 1) x u16 SYS+0x5F4 samples at 0x413F92E8 + tr x 0x927C0: row = (L x H - v) >> s clamped to H - 3 (H / 2 - 3 with SYS+0x2AA set in the first window), cells row - 1 / row / row + 1 get L - frac / 0x40 / frac (frac = (L x H - v) & (L - 1)). The covered rows are kept per column (lowest - 1 at 0x42E63D24, highest + 1 at 0x42E63528). After dcache_clean_inv_all, every covered cell gets 5 x segments + 20, then up to 4 smoothing passes (fewer for short runs; with touch byte 0x42E68BB0+0x15 and byte 0x42E61FF8 = 1 the pass count is 0x927C0 (SYS+0x1B0 = 2) or 0x124F80 over the summed run lengths, at most 4). The divisor SYS+0x7B0 becomes the largest count of the columns x0 + 2 .. x1 - 3 divided by 75.
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

void dcache_clean_inv_all(void);

void trace_intensity_accumulate_lines(u32 tr)
{
    u32 h, lh, m, nseg, np, on, k, coloff, colbuf, base, add, hh, mx, v, n;
    int x0, x1, lim, ymax, tot, shift, x, lo1, hi1, half, s, a, b;
    volatile unsigned short *p;
    tr &= 0xFF;
    h = H16(SYS + 0xA);
    lh = W32(SYS + 0x1C0) * h;
    m = mode_not_0_or_1(tr);
    nseg = H16(SYS + 0x5F4);
    fill_u16((unsigned short *)COLB, 0, 0x95B50);
    h = H16(SYS + 0xA);
    if (B8(SYS + 0x2AA) && m == 0)
        ymax = (int)(h >> 1) - 3;
    else
        ymax = (int)h - 3;
    if (m) {
        np = W32(SYS + 0x610);
        x0 = H16(SYS + 0x16);
        x1 = H16(SYS + 0x18);
        on = H16(SYS + 0x1A);
    } else {
        x0 = H16(SYS + 0x10);
        x1 = H16(SYS + 0x12);
        np = W32(SYS + 0x5F8);
        on = H16(SYS + 0x14);
    }
    if (on == 0)
        return;
    k = 1 + 2 * np;
    lim = x1 - 1;
    tot = cols_stamp(tr, x0, lim, h, ymax, nseg, k, lh);
    dcache_clean_inv_all();
    W32(SYS + 0x7B0) = 0;
    add = 5 * nseg + 20;
    shift = 4;
    if (B8(0x42E68BB0u + 0x15) && B8(0x42E61FF8u) == 1) {
        if (B8(SYS + 0x1B0) == 2)
            shift = tot ? 0x927C0 / tot : 0;
        else
            shift = tot ? 0x124F80 / tot : 0;
        if (shift > 4)
            shift = 4;
    }
    if (x0 < lim) {
        hh = H16(SYS + 0xA);
        coloff = hh * (u32)x0;
        colbuf = COLB + 0x4B0 * (u32)x0;
        base = HITS(tr);
        for (x = x0; x < lim; x++) {
            col_fill(base, colbuf, coloff, x, add);
            lo1 = (int)H16(MINL + 2 * (u32)x) - 1;
            hi1 = (int)H16(MAXL + 2 * (u32)x) + 1;
            half = (hi1 - lo1 - 2) / 2;
            s = shift;
            while (half < (int)(shl(1, s - 1) << 1))
                s--;
            for (; s > 0; s--)
                col_smooth(base, colbuf, coloff, lo1, hi1, s);
            colbuf += 0x4B0;
            coloff += hh;
        }
    }
    mx = 0;
    a = x0 + 2;
    b = x1 - 2;
    if (a < b) {
        hh = H16(SYS + 0xA);
        coloff = hh * (u32)a;
        for (x = a; x < b; x++) {
            if (H16(MINL + 2 * (u32)x) < H16(MAXL + 2 * (u32)x)) {
                p = (volatile unsigned short *)(HITS(tr) + 2 * (coloff + H16(MINL + 2 * (u32)x)));
                n = (u32)H16(MAXL + 2 * (u32)x) - H16(MINL + 2 * (u32)x);
                do {
                    v = *p++;
                    if (v != 0 && v >= mx)
                        mx = v;
                } while (--n);
            }
            coloff += hh;
        }
    }
    W32(SYS + 0x7B0) = mx / 75;
}
