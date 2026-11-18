# Zoom resampling of a channel (nearest, cubic spline or windowed sinc) and the intensity
# (persistence) drawing of a trace from its hit-count buffer.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="trace"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define F32(a) (*(volatile float *)(a))
"""

SPL = """
#define SPL_B 0x42D6EB54u
#define SPL_C 0x42D823D4u
#define SPL_D 0x42D95C54u
"""

wr(0x4005CEA4, 212, "spline_eval",
   "Evaluates a cubic spline at x (s0): segment i = *idx if 0 <= i < n - 1 and xk[i] <= x <= xk[i + 1], otherwise found by bisection over xk[0..n]; *idx = i and the result is a[i] + dx (b[i] + dx (c[i] + dx d[i])) with dx = x - xk[i]. "
   "The first argument is not used (callers pass the output count).", COM + """
float spline_eval(u32 unused, int n, const volatile float *xk, const volatile float *a, const volatile float *b,
                  const volatile float *c, const volatile float *d, volatile int *idx, float x)
{
    int i = *idx, lo, hi, mid;
    float v, dx;
    (void)unused;
    if ((u32)i >= (u32)(n - 1) || i < 0)
        i = 0;
    if (x < xk[i] || xk[i + 1] < x) {
        lo = 0;
        hi = n;
        do {
            mid = (hi + lo) / 2;
            v = xk[mid];
            if (x < v)
                hi = mid;
            if (x >= v)
                lo = mid;
        } while (lo + 1 < hi);
        i = lo;
    }
    *idx = i;
    dx = x - xk[i];
    return a[i] + (b[i] + (c[i] + dx * d[i]) * dx) * dx;
}
""")

wr(0x4005CF78, 208, "spline_resample",
   "Cubic spline through the n points (xk, yk): coefficients from 0x4005C95C (natural ends: flags 0, end slopes 0.0) into b / c / d = 0x42D6EB54 / 0x42D823D4 / 0x42D95C54, then yout[j] = spline_eval(xin[j]) for j < m, keeping the segment index between calls.",
   COM + SPL + """
void func_4005c95c(int n, int f0, int f1, const volatile float *xk, const volatile float *yk, volatile float *b,
                   volatile float *c, volatile float *d, volatile u32 *work, float s0, float s1);
float spline_eval(u32 unused, int n, const volatile float *xk, const volatile float *a, const volatile float *b,
                  const volatile float *c, const volatile float *d, volatile int *idx, float x);

void spline_resample(int n, const volatile float *xk, const volatile float *yk, u32 m, const volatile float *xin,
                     volatile float *yout)
{
    volatile u32 work = 0;
    volatile int idx = 0;
    u32 k;
    func_4005c95c(n, 0, 0, xk, yk, (volatile float *)SPL_B, (volatile float *)SPL_C, (volatile float *)SPL_D, &work,
                  0.0f, 0.0f);
    for (k = m; k; k--)
        *yout++ = spline_eval(m, n, xk, yk, (volatile float *)SPL_B, (volatile float *)SPL_C,
                              (volatile float *)SPL_D, &idx, *xin++);
}
""")

wr(0x4005D048, 472, "floating_point_interpolation_resampling",
   "Spline zoom of the u16 workspace ws around index half: with k = SYS+0x554 (current zoom) and m the target, c = (len x k / m) x 12 / 10 input samples starting c/2 before half are turned into floats (v - 128 x SYS+0x1C0) / (128 x SYS+0x1C0) at 0x42DBCD54; "
   "cnt = m x c / k outputs at x = j x k / m (knots x = j at 0x42DA94D4, positions at 0x42DD05D4) go through spline_resample into 0x42DE3E54 and back to u16 (v + 1) x scale, negative values to 0, at 0x416431E8; "
   "then len samples, centred and moved back by the 64-bit stack argument, are copied to ws from half - len/2.", COM + """
void spline_resample(int n, const volatile float *xk, const volatile float *yk, u32 m, const volatile float *xin,
                     volatile float *yout);
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);

#define FY 0x42DBCD54u
#define FX 0x42DA94D4u
#define FQ 0x42DD05D4u
#define FO 0x42DE3E54u
#define OUT16 0x416431E8u

void floating_point_interpolation_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift)
{
    u32 k = W32(SYS + 0x554), lv = W32(SYS + 0x1C0), sb = lv << 7, cnt, src, dst, i;
    float ratio = (float)m / (float)k, scale, inv;
    long long t;
    int c, j, v;
    double sc = (double)lv * 128.0;
    t = (long long)((unsigned long long)len * k) / (long long)m;
    c = (int)((long long)((unsigned long long)t * 12) / 10);
    scale = (float)sc;
    src = ws + half * 2 - ((u32)(c + (int)((u32)c >> 31)) & ~1u);
    dst = ws + half * 2 - (len & ~1u);
    cnt = (u32)c * m / k;
    if (c >= 0) {
        j = 0;
        do {
            v = (int)(H16(src) - sb);
            src += 2;
            F32(FY + 4 * j) = (float)v / scale;
            j++;
        } while (c >= j);
    }
    if ((int)cnt > 0) {
        inv = 1.0f / ratio;
        for (j = 0; j < (int)cnt; j++) {
            F32(FX + 4 * j) = (float)j;
            F32(FQ + 4 * j) = (float)j * inv;
        }
    }
    spline_resample(c, (const volatile float *)FX, (const volatile float *)FY, cnt, (const volatile float *)FQ,
                    (volatile float *)FO);
    if ((int)cnt > 0)
        for (i = 0; i < cnt; i++) {
            v = (int)((F32(FO + 4 * i) + 1.0f) * scale);
            if (v < 0)
                v = 0;
            H16(OUT16 + 2 * i) = (unsigned short)v;
        }
    copy_u16((unsigned short *)dst,
             (const unsigned short *)(OUT16 + ((cnt - len + ((cnt - len) >> 31)) & ~1u) - ((u32)shift << 1)), len);
}
""")

wr(0x4005D2E4, 372, "fixed_point_weighted_or_filtered_resampling",
   "Windowed-sinc zoom of the u16 workspace ws around index half: k = SYS+0x554, c = (len x k / m) x 12 / 10 input samples starting c/2 before half, cnt = m x c / k outputs; output o sums x[j] x w(p - 10000 j) over the c inputs, "
   "with p = o x (10000 / (m / k)) and w(0) = 10000, w(t) = sine_table_lookup(t) / t, then divides by 10000 (negative results to 0) into 0x416431E8; finally (u16) len samples, centred and moved back by the 64-bit stack argument, are copied to ws from half - len/2.", COM + """
int sine_table_lookup(int x);
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);

#define OUT16 0x416431E8u

void fixed_point_weighted_or_filtered_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift)
{
    u32 k = W32(SYS + 0x554), step, cnt, src, dst, l16, out, ph, acc, o;
    long long t;
    int c, j, ph2, w, r;
    t = (long long)((unsigned long long)len * k) / (long long)m;
    step = 10000u / (m / k);
    c = (int)((long long)((unsigned long long)t * 12) / 10);
    src = ws + half * 2 - ((u32)(c + (int)((u32)c >> 31)) & ~1u);
    cnt = (u32)c * m / k;
    l16 = len & 0xFFFF;
    dst = ws + half * 2 - (l16 & ~1u);
    if ((int)cnt > 0) {
        out = OUT16;
        ph = 0;
        for (o = cnt; o; o--) {
            acc = 0;
            if (c > 0) {
                ph2 = (int)ph;
                for (j = 0; j < c; j++) {
                    if (ph2 == 0)
                        w = 10000;
                    else
                        w = sine_table_lookup(ph2) / ph2;
                    acc += (u32)w * H16(src + 2 * (u32)j);
                    ph2 -= 10000;
                }
            }
            r = (int)acc / 10000;
            if (r < 0)
                r = 0;
            H16(out) = (unsigned short)r;
            out += 2;
            ph += step;
        }
    }
    copy_u16((unsigned short *)dst,
             (const unsigned short *)(OUT16 + ((cnt - l16 + ((cnt - l16) >> 31)) & ~1u) - ((u32)shift << 1)), l16);
}
""")

wr(0x4005C7BC, 348, "workspace_copy_and_resampling",
   "Zoom of channel ch's u16 trace src while SYS+0x554 < n: the len samples centred on half (from half - len/2, plus a tail of 15 / 7 / 4 / 2 / 1 for CH2 at time-base index SYS+0x588 = 12..16) are copied to the workspace 0x41A13AE8, "
   "resampled by the method of byte SYS+0x51C (CH1) / +0x51D (CH2): 0 nearest neighbour, 1 cubic spline, other windowed sinc (shift 0), and copied back (CH2 shifted by the tail).", COM + """
void floating_point_interpolation_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift);
void fixed_point_weighted_or_filtered_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift);

#define WS 0x41A13AE8u

void workspace_copy_and_resampling(u32 ch, u32 src, u32 half, u32 len, u32 n)
{
    int lo, hi, e, i;
    u32 mode, tail = 0;
    if (W32(SYS + 0x554) >= n)
        return;
    lo = (int)(half - (len >> 1));
    mode = ch == 0 ? B8(SYS + 0x51C) : B8(SYS + 0x51D);
    hi = lo + (int)len;
    if (ch == 1)
        switch (B8(SYS + 0x588)) {
        case 12: tail = 15; break;
        case 13: tail = 7; break;
        case 14: tail = 4; break;
        case 15: tail = 2; break;
        case 16: tail = 1; break;
        }
    e = (int)tail + hi;
    for (i = lo; i < e; i++)
        H16(WS + 2 * (u32)i) = H16(src + 2 * (u32)i);
    if (mode == 0)
        simple_integer_index_mapping_nearest_neighbour(WS, half, len, n, 0, 0);
    else if (mode == 1)
        floating_point_interpolation_resampling(WS, half, len, n, 0);
    else
        fixed_point_weighted_or_filtered_resampling(WS, half, len, n, 0);
    for (i = lo; i < hi; i++)
        H16(src + 2 * (u32)i) = H16(WS + 2 * (tail + (u32)i));
}
""")

PERS = """
u32 mode_not_0_or_2(u32 v);
void dcache_clean_inv_all(void);
void fb_present_rect(u32 x0, u32 y0, u32 x1, u32 y1);

#define HITS(tr) (0x400954A4u + (tr) * 0x12BE70u)
#define FBP (*(volatile u32 *)0x40095498u)
#define PAL_A 0x42E45864u
#define PAL_B 0x42E47B8Cu
"""

wr(0x4005DDD4, 392, "trace_intensity_draw",
   "Intensity (persistence) view of trace tr: for each of the u16 SYS+8 columns and SYS+0xA rows of the plot (origin u16 SYS+4 + 1, SYS+6), a non-zero hit count v (u16, 0x400954A4 + tr x 0x12BE70, column-major) is drawn as palette[2 + sqrt(byte SYS+0x654) x byte SYS+0x654 x 22 / 1000 + ((v / SYS+0x7B0) x 100 + 1) / 110]. "
   "Palette: 0x42E45864 (byte SYS+0x653 = 0) or 0x42E47B8C, offset 6000 bytes in the single-trace mode (byte SYS+0x652), else 3000 x mode_not_0_or_2(tr); in that second case with SYS+0x653 set, SYS+0x7B0 is divided by 3 (stored). The D-cache is cleaned first.", COM + PERS + """
void trace_intensity_draw(u32 tr)
{
    u32 x1 = H16(SYS + 4) + 1, y0 = H16(SYS + 6), cols = H16(SYS + 8), rows = H16(SYS + 0xA), pal, k, base, out,
        col, row, off, src, dst, v, q, idx;
    int po;
    if (B8(SYS + 0x652)) {
        pal = (B8(SYS + 0x653) ? PAL_B : PAL_A) + 6000;
    } else {
        po = (int)(short)3000 * (int)(short)mode_not_0_or_2(tr);
        if (B8(SYS + 0x653)) {
            pal = PAL_B + (u32)po;
            W32(SYS + 0x7B0) = W32(SYS + 0x7B0) / 3;
        } else
            pal = PAL_A + (u32)po;
    }
    k = B8(SYS + 0x654);
    base = 2 + (u32)SQRT_T((double)k) * k * 22 / 1000;
    dcache_clean_inv_all();
    out = FBP + x1 * 2 + y0 * 1022 * 2;
    off = 0;
    for (col = cols; col; col--, out += 2, off += rows) {
        if (rows == 0)
            continue;
        src = HITS(tr & 0xFF) + off * 2;
        dst = out;
        for (row = rows; row; row--, dst += 0x7FC, src += 2) {
            v = H16(src);
            if (v == 0)
                continue;
            q = v / W32(SYS + 0x7B0) * 100;
            idx = base + ((u32)(((unsigned long long)q * 0x94F2094Fu + 0x94F2094Fu) >> 32) >> 6);
            H16(dst) = H16(pal + idx * 2);
        }
    }
}
""".replace("void trace_intensity_draw", "typedef double (*sqrt_fn)(double);\n#define SQRT_T ((sqrt_fn)0x4003DF75u)   /* sqrt at 0x4003DF74 is Thumb */\n\nvoid trace_intensity_draw"))

wr(0x4005DF5C, 332, "trace_intensity_draw_window",
   "Intensity view over the capture window, only when SYS+0x6FC >= SYS+0x708: columns SYS+0x700 .. SYS+0x700 + u16 SYS+0x6F8 of trace tr drawn as palette[16 + (v / SYS+0x7B0) x 1000 / 1333] (palette as in trace_intensity_draw, without the SYS+0x7B0 update), "
   "then fb_present_rect over the window. (The symbol table splits this function at 0x4005E0AC; the entry here covers both pieces.)", COM + PERS + """
void trace_intensity_draw_window(u32 tr)
{
    u32 w700, x0, y0, rows, pal, out, c0, c1, n, off, src, dst, row, v, q, ax, bx, cy;
    int po;
    if (W32(SYS + 0x6FC) < W32(SYS + 0x708))
        return;
    w700 = W32(SYS + 0x700);
    x0 = H16(SYS + 4);
    c1 = H16(SYS + 0x6F8) + w700;
    ax = w700 + x0 + 1;
    bx = H16(SYS + 0x6F8) + ax;
    y0 = H16(SYS + 6);
    rows = H16(SYS + 0xA);
    cy = rows + y0;
    if (B8(SYS + 0x652)) {
        pal = (B8(SYS + 0x653) ? PAL_B : PAL_A) + 6000;
    } else {
        po = (int)(short)3000 * (int)(short)mode_not_0_or_2(tr);
        pal = (B8(SYS + 0x653) ? PAL_B : PAL_A) + (u32)po;
    }
    out = FBP + (x0 + 1) * 2 + y0 * 1022 * 2;
    c0 = w700;
    if (c0 < c1) {
        out += c0 * 2;
        off = rows * c0;
        for (n = c1 - c0; n; n--, out += 2, off += rows) {
            if (rows == 0)
                continue;
            src = HITS(tr & 0xFF) + off * 2;
            dst = out;
            for (row = rows; row; row--, dst += 0x7FC, src += 2) {
                v = H16(src);
                if (v == 0)
                    continue;
                q = v / W32(SYS + 0x7B0) * 1000;
                H16(dst) = H16(pal + ((u32)(((unsigned long long)q * 0xC4A83BECu) >> 32) >> 10) * 2 + 0x20);
            }
        }
    }
    fb_present_rect(ax & 0xFFFF, y0, bx & 0xFFFF, cy & 0xFFFF);
}
""")
