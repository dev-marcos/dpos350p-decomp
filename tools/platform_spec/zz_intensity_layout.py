# The two other hit accumulators of the intensity (persistence) view: the line mode with column
# smoothing and the capture-window version; and the layout of the level view.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="trace"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
"""

ACC = """
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
"""

wr(0x4005E12C, 1764, "trace_intensity_accumulate_lines",
   "Line-mode hit accumulation of trace tr (u8): clears the column flags 0x426BFF94 (0x95B50 u16), then for the columns x0 .. x1 - 2 of the plot (u16 SYS+0x10 / +0x12, points W32 SYS+0x5F8, gate u16 SYS+0x14; "
   "the second window SYS+0x16 / +0x18 / +0x610 / +0x1A when mode_not_0_or_1(tr)) stamps each of the (2 points + 1) x u16 SYS+0x5F4 samples at 0x413F92E8 + tr x 0x927C0: "
   "row = (L x H - v) >> s clamped to H - 3 (H / 2 - 3 with SYS+0x2AA set in the first window), cells row - 1 / row / row + 1 get L - frac / 0x40 / frac (frac = (L x H - v) & (L - 1)). "
   "The covered rows are kept per column (lowest - 1 at 0x42E63D24, highest + 1 at 0x42E63528). After dcache_clean_inv_all, every covered cell gets 5 x segments + 20, "
   "then up to 4 smoothing passes (fewer for short runs; with touch byte 0x42E68BB0+0x15 and byte 0x42E61FF8 = 1 the pass count is 0x927C0 (SYS+0x1B0 = 2) or 0x124F80 over the summed run lengths, at most 4). "
   "The divisor SYS+0x7B0 becomes the largest count of the columns x0 + 2 .. x1 - 3 divided by 75.",
   COM + ACC + """
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
""")

wr(0x4005E838, 1412, "trace_intensity_accumulate_window",
   "Capture-window version of trace_intensity_accumulate_lines for trace tr: only when SYS+0x6FC >= SYS+0x708; the columns are c0 = SYS+0x700 .. c0 + u16 SYS+0x6F8 - 1 and only their flags (from 0x426BFF94 + 2 c0, u16 SYS+0x6F8 entries) are cleared. "
   "Same stamping, covered-cell increment 5 x segments + 20 and always four smoothing passes; the divisor SYS+0x7B0 becomes 45 x W32 SYS+0x5F8 / 50.",
   COM + ACC + """
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
""")

wr(0x400612FC, 632, "levels_view_layout",
   "Unless touch byte 0x42E68BB0+0x17 is set, lays out the level view in block 0x42E68668 from its bytes +1 / +2 (the two side panels): the plot box +0xB0 / +0xB4 / +0xB8 (width, also halved into +0xBC) / +0xC0 .. +0xD0 and the panel boxes +0xD4 .. +0xE0 / +0xE4 .. +0xF0. "
   "The FFT size +0xF8 becomes 0x1000 / 0x2000 / 0x4000 / 0x8000 for mode +0xF4 = 0..3 (kept otherwise), the sample pointer 0x42E68CB0 = 0x42CC64B4 + 2 x size, SYS+0x3C = 0, SYS+0x54E = 1. "
   "From the s64 sample rate SYS+0x538: bin width +0x100 = rate / 1000 / size, top +0x108 = (rate / 1000, unsigned) / 2, centre +0x118 = (top + bin) / 2, span +0x110 = top - bin.",
   COM + """
#define BLK 0x42E68668u

unsigned long long __aeabi_uldivmod(unsigned long long n, unsigned long long d);

void levels_view_layout(void)
{
    u32 n, f1, f2, x, y, w, c0, c4, c8, cc, d0;
    long long rate, bin, top;
    if (B8(0x42E68BB0u + 0x17))
        return;
    n = W32(BLK + 0xF8);
    switch (W32(BLK + 0xF4)) {
    case 0: n = 0x1000; break;
    case 1: n = 0x2000; break;
    case 2: n = 0x4000; break;
    case 3: n = 0x8000; break;
    }
    f1 = B8(BLK + 1);
    x = W32(BLK + 0xB0);
    y = W32(BLK + 0xB4);
    w = W32(BLK + 0xB8);
    c0 = W32(BLK + 0xC0);
    c4 = W32(BLK + 0xC4);
    c8 = W32(BLK + 0xC8);
    cc = W32(BLK + 0xCC);
    d0 = W32(BLK + 0xD0);
    f2 = B8(BLK + 2);
    if (!f1 && !f2) {
        x = 0x34; y = 0x30; w = 0x3C6; c0 = 0x1E0; c4 = 0xA1; c8 = 6; cc = 0x50; d0 = 6;
    } else if (f1) {
        x = 0x34; y = 0x30; w = 0x3C6;
        if (!f2) {
            cc = 0x2F;
            W32(BLK + 0xD4) = 0x34;
            c0 = 0x11A; c4 = 0xA1; c8 = 6; d0 = 6;
            W32(BLK + 0xD8) = 0x163;
            W32(BLK + 0xDC) = 0x3C6;
            W32(BLK + 0xE0) = 0xAB;
        } else {
            W32(BLK + 0xD4) = 0x34;
            c0 = 0x8F;
            W32(BLK + 0xE4) = 0x34;
            W32(BLK + 0xD8) = 0xD8;
            c4 = 0xA1;
            W32(BLK + 0xDC) = 0x3C6;
            c8 = 6;
            W32(BLK + 0xE0) = 0x8F;
            cc = 0x18; d0 = 6;
            W32(BLK + 0xE8) = 0x180;
            W32(BLK + 0xEC) = 0x3C6;
            W32(BLK + 0xF0) = 0x8F;
        }
    } else {
        cc = 0x2F; w = 0x3C6; x = 0x34; y = 0x30; c0 = 0x11A;
        W32(BLK + 0xE4) = 0x34;
        c4 = 0xA1; c8 = 6; d0 = 6;
        W32(BLK + 0xE8) = 0x163;
        W32(BLK + 0xEC) = 0x3C6;
        W32(BLK + 0xF0) = 0xAB;
    }
    W32(BLK + 0xCC) = cc;
    W32(BLK + 0xD0) = d0;
    W32(BLK + 0xB0) = x;
    W32(BLK + 0xB4) = y;
    W32(BLK + 0xC0) = c0;
    W32(BLK + 0xC4) = c4;
    W32(BLK + 0xC8) = c8;
    B8(SYS + 0x3C) = 0;
    B8(SYS + 0x54E) = 1;
    W32(0x42E68CB0u) = 0x42CC64B4u + n * 2;
    W32(BLK + 0xF8) = n;
    W32(BLK + 0xB8) = w;
    W32(BLK + 0xBC) = (u32)((int)w / 2);
    rate = S64(SYS + 0x538);
    bin = __aeabi_ldivmod(__aeabi_ldivmod(rate, 1000), (long long)(int)W32(BLK + 0xF8));
    top = (long long)(__aeabi_uldivmod((unsigned long long)rate, 1000) >> 1);
    S64(BLK + 0x108) = top;
    S64(BLK + 0x100) = bin;
    S64(BLK + 0x118) = __aeabi_ldivmod(top + bin, 2);
    S64(BLK + 0x110) = S64(BLK + 0x108) - S64(BLK + 0x100);
}
""", src="ui")

wr(0x40061A70, 820, "levels_samples_window",
   "Unless touch byte 0x42E68BB0+0x17 is set, fits the visible part of the level samples to the plot width w = s32 0x42E68668+0xB8: with n = +0xF8, bin = s64 +0x100 and span = s64 +0x110, the first / last index i = (((s64 +0x120 / +0x128) - bin) x n / 2) / span; "
   "clears 0x42CC64B4 (n + 2 u16) and n u16 at 0x42CC64B4 + 2n + 2 (n / 2) - 10; c = i1 - i0 + 1 samples from *(0x42E68CB0) + 2 i0 + 2. "
   "If w < c: peak_hold_decimate(src, w, c / w) and copy w samples back to the start of the buffer. "
   "Otherwise the samples (minus 0x8000) are centred at index 500000 of the zoom workspace 0x416431E8 with 100 copies of each end sample beyond them, zoomed to w points by the nearest-neighbour (m = w x SYS+0x554 / c below 3 x SYS+0x554) or the spline resampler, "
   "offset back by 0x8000 into 0x4191F8A8 and copied to the buffer start.",
   COM + """
#define BLK 0x42E68668u
#define SB 0x42CC64B4u
#define PTR 0x42E68CB0u
#define OUT 0x416431E8u
#define MID (OUT + 0xF4240u)
#define FIN 0x4191F8A8u

unsigned long long __aeabi_uldivmod(unsigned long long n, unsigned long long d);
void fill_u16(unsigned short *d, unsigned short v, u32 n);
void peak_hold_decimate(volatile short *buf, u32 n, double step);
void floating_point_interpolation_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift);

void levels_samples_window(void)
{
    long long bin, span, i0, i1, c, k;
    u32 n, nn, src, m, h2, wh;
    int w;
    volatile unsigned short *d, *p;
    if (B8(0x42E68BB0u + 0x17))
        return;
    n = W32(BLK + 0xF8);
    bin = S64(BLK + 0x100);
    span = S64(BLK + 0x110);
    i0 = __aeabi_ldivmod(__aeabi_ldivmod((S64(BLK + 0x120) - bin) * (long long)(int)n, 2), span);
    i1 = __aeabi_ldivmod(__aeabi_ldivmod((S64(BLK + 0x128) - bin) * (long long)(int)n, 2), span);
    fill_u16((unsigned short *)SB, 0, n + 2);
    nn = W32(BLK + 0xF8);
    fill_u16((unsigned short *)(SB + nn * 2 + (u32)(((int)nn / 2) * 2) - 10), 0, nn);
    c = i1 - i0 + 1;
    src = W32(PTR) + (u32)i0 * 2 + 2;
    w = (int)W32(BLK + 0xB8);
    if ((long long)w < c) {
        peak_hold_decimate((volatile short *)src, (u32)w, (double)c / (double)(long long)w);
        copy_u16((unsigned short *)W32(PTR), (const unsigned short *)src, (u32)w);
        return;
    }
    m = (u32)__aeabi_uldivmod((unsigned long long)(long long)w * (unsigned long long)W32(SYS + 0x554),
                              (unsigned long long)c);
    h2 = (u32)__aeabi_ldivmod(c, 2);
    if (c >= 1) {
        d = (volatile unsigned short *)(MID - 2 * h2);
        p = (volatile unsigned short *)src;
        k = c;
        do {
            *d++ = (unsigned short)(*p++ - 0x8000);
        } while (--k);
    }
    fill_u16((unsigned short *)(MID - 2 * h2 - 200), H16(MID - 2 * h2), 100);
    fill_u16((unsigned short *)(MID + 2 * h2), H16(MID + 2 * h2 - 2), 100);
    if (m < 3 * W32(SYS + 0x554))
        simple_integer_index_mapping_nearest_neighbour(OUT, 500000, (u32)w & 0xFFFF, m, 0, 0);
    else
        floating_point_interpolation_resampling(OUT, 500000, (u32)w & 0xFFFF, m, 0LL);
    wh = (u32)__aeabi_ldivmod((long long)w, 2);
    if ((long long)w >= 1) {
        d = (volatile unsigned short *)(FIN - 2 * wh);
        p = (volatile unsigned short *)(MID - 2 * wh);
        k = w;
        do {
            *d++ = (unsigned short)((short)*p++ - 32768);
        } while (--k);
    }
    copy_u16((unsigned short *)W32(PTR), (const unsigned short *)(FIN - 2 * wh), (u32)w);
}
""")
