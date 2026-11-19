# Cubic spline coefficients, intensity palettes and hit accumulation, peak-hold decimation, the
# "Max =" panel of the level view and the FFT spur capture / cancel pair.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="trace"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define F32(a) (*(volatile float *)(a))
#define D64(a) (*(volatile double *)(a))
"""

SPUR = """
#define BLK 0x42E68668u
#define SPT 0x42E3C000u
#define SPEC 0x42D364B4u
typedef long long (*f2lz_fn)(float) __attribute__((pcs("aapcs")));
#define F2LZ(x) (((f2lz_fn)0x4006A4E9u)(x))   /* __aeabi_f2lz at 0x4006A4E8 is Thumb */

/* per mode word BLK+0xF4 = 0..3: values (float), bin indices (u16), mean (float), count (u16) */
static void spur_lists(u32 sel, u32 *vals, u32 *idxs, u32 *mean, u32 *cntp)
{
    switch (sel) {
    case 0: *vals = SPT + 0x2340; *idxs = SPT + 0x37F4; *mean = SPT + 0x14; *cntp = SPT + 6; break;
    case 1: *vals = SPT + 0x1D00; *idxs = SPT + 0x34D4; *mean = SPT + 0x10; *cntp = SPT + 4; break;
    case 2: *vals = SPT + 0x12D8; *idxs = SPT + 0x2FC0; *mean = SPT + 0xC; *cntp = SPT + 2; break;
    default: *vals = SPT + 0x18; *idxs = SPT + 0x2660; *mean = SPT + 8; *cntp = SPT; break;
    }
}
"""

wr(0x4005C95C, 1352, "spline_coeffs",
   "Cubic spline coefficients through the n points (x, y): *iflag = 1 and nothing else when n <= 1 (unsigned), *iflag = 2 when the x are not strictly increasing; n = 2 gives the straight line (c = d = 0). "
   "Otherwise the tridiagonal system is solved with h = x[i+1] - x[i] kept in d, the diagonal in b and the right-hand side in c; the end conditions are third-derivative matching (n > 3, zero for n = 3), replaced by the slope slope1 (end1 = 1) / slope2 (end2 = 1). "
   "On return y[i] + b dx + c dx^2 + d dx^3 is the spline on [x[i], x[i+1]] (the form used by spline_eval). Float arithmetic in the stock order; the return value is not set.",
   COM + """
void spline_coeffs(int n, int end1, int end2, const volatile float *x, const volatile float *y, volatile float *b,
                   volatile float *c, volatile float *d, volatile u32 *iflag, float slope1, float slope2)
{
    u32 un = (u32)n;
    int nm1 = n - 1, i, ok, ib;
    float t;
    *iflag = 0;
    if (un <= 1) {
        *iflag = 1;
        return;
    }
    ok = 1;
    for (i = 0; i < nm1; i++)
        if (x[i] >= x[i + 1])
            ok = 0;
    if (!ok) {
        *iflag = 2;
        return;
    }
    if (un <= 2) {
        float h = x[1] - x[0];
        b[0] = (y[1] - y[0]) / h;
        c[0] = 0.0f;
        d[0] = 0.0f;
        b[1] = b[0];
        c[1] = 0.0f;
        d[1] = 0.0f;
        return;
    }
    d[0] = x[1] - x[0];
    c[1] = (y[1] - y[0]) / d[0];
    for (i = 1; i < nm1; i++) {
        d[i] = x[i + 1] - x[i];
        b[i] = (d[i - 1] + d[i]) * 2.0f;
        c[i + 1] = (y[i + 1] - y[i]) / d[i];
        c[i] = c[i + 1] - c[i];
    }
    b[0] = -d[0];
    b[nm1] = -d[n - 2];
    c[0] = 0.0f;
    c[nm1] = 0.0f;
    if (n != 3) {
        c[0] = c[2] / (x[3] - x[1]) - c[1] / (x[2] - x[0]);
        c[nm1] = c[n - 2] / (x[nm1] - x[n - 3]) - c[n - 3] / (x[n - 2] - x[n - 4]);
        c[0] = c[0] * d[0] * d[0] / (x[3] - x[0]);
        c[nm1] = -(c[nm1] * d[n - 2] * d[n - 2] / (x[nm1] - x[n - 4]));
    }
    if (end1 == 1) {
        b[0] = (x[1] - x[0]) * 2.0f;
        c[0] = (y[1] - y[0]) / (x[1] - x[0]) - slope1;
    }
    if (end2 == 1) {
        b[nm1] = (x[nm1] - x[n - 2]) * 2.0f;
        c[nm1] = slope2 - (y[nm1] - y[n - 2]) / (x[nm1] - x[n - 2]);
    }
    for (i = 1; (u32)i < un; i++) {
        t = d[i - 1] / b[i - 1];
        b[i] = b[i] - t * d[i - 1];
        c[i] = c[i] - t * c[i - 1];
    }
    c[nm1] = c[nm1] / b[nm1];
    for (ib = 0; ib < nm1; ib++) {
        i = n - ib - 2;
        c[i] = (c[i] - d[i] * c[i + 1]) / b[i];
    }
    b[nm1] = (y[nm1] - y[n - 2]) / d[n - 2] + (c[n - 2] + c[nm1] * 2.0f) * d[n - 2];
    for (i = 0; i < nm1; i++) {
        b[i] = (y[i + 1] - y[i]) / d[i] - (c[i + 1] + c[i] * 2.0f) * d[i];
        d[i] = (c[i + 1] - c[i]) / d[i];
        c[i] = c[i] * 3.0f;
    }
    c[nm1] = c[nm1] * 3.0f;
    d[nm1] = d[n - 2];
}
""")

wr(0x4005DB9C, 568, "intensity_palettes_init",
   "Builds the RGB565 palettes of the intensity view. Palette A (0x42E45864, three 1500-entry parts): yellow 0xFFE0 / cyan 0x7FF / green 0x7E0 fills, each starting with a 64-step ramp from black. "
   "Palette B (0x42E47B8C): a 192-step blue - cyan - green - yellow - red ramp over a red 0xF800 fill of 1000 entries, copied to its parts at +3000 and +6000 bytes. "
   "Then two u32 colour tables, both filled with 0xFFFF first: 0x42E56C30 = the ramps 0xFF .. 0xFFFF .. 0xFF00 .. 0xFFFF00 .. 0xFF0000 (1021 words, past its 1000-word fill) and 0x42E583A0 = 0x33 .. 0xFF .. 0xFFFF (460 words).",
   COM + """
#define PAL_A 0x42E45864u
#define PAL_B 0x42E47B8Cu
#define TAB_A 0x42E56C30u
#define TAB_B 0x42E583A0u

void fill_u16(unsigned short *d, unsigned short v, u32 n);
void fill_u32(u32 *d, u32 v, u32 n);

void intensity_palettes_init(void)
{
    volatile unsigned short *pa = (volatile unsigned short *)PAL_A, *pb = (volatile unsigned short *)PAL_B;
    volatile u32 *t;
    int i;
    u32 v;
    fill_u16((unsigned short *)PAL_A, 0xFFE0, 1500);
    for (i = 0; i < 64; i++)
        pa[i] = (unsigned short)(((i / 2) << 11) + (i << 5));
    fill_u16((unsigned short *)(PAL_A + 3000), 0x7FF, 1500);
    for (i = 0; i < 64; i++)
        pa[1500 + i] = (unsigned short)((i / 2) + (i << 5));
    fill_u16((unsigned short *)(PAL_A + 6000), 0x7E0, 1500);
    for (i = 0; i < 64; i++)
        pa[3000 + i] = (unsigned short)(i << 5);
    fill_u16((unsigned short *)PAL_B, 0xF800, 1000);
    i = 0;
    for (v = 0x1F; v <= 0x7FF; v += 0x20)
        pb[i++] = (unsigned short)v;
    for (v = 0x7FF; v >= 0x7E0; v--)
        pb[i++] = (unsigned short)v;
    for (v = 0x7E0; v <= 0xFFE0; v += 0x800)
        pb[i++] = (unsigned short)v;
    for (v = 0xFFE0; v >= 0xF800; v -= 0x20)
        pb[i++] = (unsigned short)v;
    copy_u16((unsigned short *)(PAL_B + 3000), (const unsigned short *)PAL_B, 1000);
    copy_u16((unsigned short *)(PAL_B + 6000), (const unsigned short *)PAL_B, 1000);
    t = (volatile u32 *)TAB_A;
    fill_u32((u32 *)TAB_A, 0xFFFF, 1000);
    i = 0;
    t[0] = 0xFF;
    for (v = 0x1FF; v <= 0xFFFF; v += 0x100)
        t[++i] = v;
    for (v = 0xFFFE; v >= 0xFF00; v--)
        t[++i] = v;
    for (v = 0x1FF00; v <= 0xFFFF00; v += 0x10000)
        t[++i] = v;
    for (v = 0xFFFE00; v >= 0xFF0000; v -= 0x100)
        t[++i] = v;
    t = (volatile u32 *)TAB_B;
    fill_u32((u32 *)TAB_B, 0xFFFF, 1000);
    i = 0;
    t[0] = 0x33;
    for (v = 0x34; v <= 0xFF; v++)
        t[++i] = v;
    for (v = 0x1FF; v <= 0xFFFF; v += 0x100)
        t[++i] = v;
}
""")

wr(0x4005EDC0, 540, "trace_intensity_accumulate",
   "Adds the current capture of trace tr (u8) to its hit-count buffer 0x400954A4 + tr x 0x12BE70 (u16, H = u16 SYS+0xA entries per column), only when u16 SYS+0x14 is set. "
   "For each of the u16 SYS+0x5F4 segments (W32 SYS+0x2B0 points each, from offset 0) and each point j = 10 .. points - 11, with a / b the u16 values at 0x411AF3E8 / +0x927C0: "
   "column (H16(SYS+8) - H) / 2 + (a >> s), row ((L x H - b) >> s) - 1 with s = SYS+0x1BC, L = SYS+0x1C0; the 3 x 3 cells around it get +5, the centre +15. "
   "Then the largest non-zero count over the columns u16 SYS+0x10 + 1 .. u16 SYS+0x12 - 2, rows 20 .. H - 41, divided by 200, becomes the intensity divisor SYS+0x7B0.",
   COM + """
#define HITS(tr) (0x400954A4u + (tr) * 0x12BE70u)
#define PTS 0x411AF3E8u

#define INC(i, k) (H16(base + 2u * (u32)(i)) = (unsigned short)(H16(base + 2u * (u32)(i)) + (k)))

void trace_intensity_accumulate(u32 tr)
{
    u32 h = H16(SYS + 0xA), lh, y0, y1, nseg, lim, o, j, a, b, base, mx, col, v;
    int off, sh, idx, hm, r, k, cnt;
    volatile unsigned short *p;
    lh = W32(SYS + 0x1C0) * h;
    off = ((int)H16(SYS + 8) - (int)h) / 2;
    y1 = H16(SYS + 0x12);
    y0 = H16(SYS + 0x10);
    if (H16(SYS + 0x14) == 0)
        return;
    tr &= 0xFF;
    nseg = H16(SYS + 0x5F4);
    if (nseg) {
        base = HITS(tr);
        lim = W32(SYS + 0x2B0) - 10;
        sh = (int)W32(SYS + 0x1BC);
        o = 0;
        do {
            u32 step = W32(SYS + 0x2B0);
            if (lim >= 11) {
                p = (volatile unsigned short *)(PTS + 2 * o + 0x14);
                j = 10;
                do {
                    a = p[0];
                    b = H16((u32)p + 0x927C0);
                    p++;
                    j++;
                    idx = (int)(h * (u32)(off + ((int)a >> sh))) + ((int)(lh - b) >> sh) - 1 - (int)h;
                    INC(idx, 5);
                    INC(idx + 1, 5);
                    INC(idx + 2, 5);
                    idx += (int)h;
                    INC(idx, 5);
                    INC(idx + 1, 15);
                    INC(idx + 2, 5);
                    idx += (int)h;
                    INC(idx, 5);
                    INC(idx + 1, 5);
                    INC(idx + 2, 5);
                } while (j < lim);
            }
            o += step;
        } while (--nseg);
    }
    hm = (int)h - 40;
    r = (int)y0 + 1;
    mx = 0;
    if (r < (int)y1 - 1) {
        col = h * (u32)r;
        cnt = (int)y1 - 1 - (int)y0 - 1;
        do {
            if (hm > 20) {
                p = (volatile unsigned short *)(HITS(tr) + col * 2 + 0x28);
                k = hm - 20;
                do {
                    v = *p++;
                    if (v != 0 && v >= mx)
                        mx = v;
                } while (--k);
            }
            col += h;
        } while (--cnt);
    }
    W32(SYS + 0x7B0) = mx / 200;
}
""")

wr(0x40061608, 112, "peak_hold_decimate",
   "In-place peak decimation of n s16 samples: output k = the maximum (at least -28671) of the samples start .. (u32)pos, with pos = k x step (d0) and start the previous end; nothing when n = 0.",
   COM + """
void peak_hold_decimate(volatile short *buf, u32 n, double step)
{
    double pos = 0.0;
    u32 start = 0, end, cnt;
    volatile short *out = buf, *p;
    int mx;
    if (n == 0)
        return;
    do {
        end = (u32)pos;
        mx = -28671;
        cnt = end - start + 1;
        p = buf + start;
        while (cnt) {
            if (*p >= mx)
                mx = *p;
            p++;
            cnt--;
        }
        *out++ = (short)mx;
        start = end;
        pos += step;
    } while (--n);
}
""")

wr(0x40062A18, 240, "levels_max_panel_draw",
   "Unless touch byte 0x42E68BB0+0x17 is set, draws the 'Max = ' line of the level view at (W32 0x42E68668+0xB0 + 8, +0xB4 + 8) in colour 0xDDDDDD: the text, the frequency (s32 +8, drawn then measured), "
   "after a gap of 20 the level +0x10 in dBmV (level_label_dbmv flag 1) and then the voltage +0x18 x 1000 (voltage_uv_draw), each placed after the font advance u16 0x42E68BE4.",
   COM + """
#define BLK 0x42E68668u
#define ADV 0x42E68BE4u

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void text_draw(u32 x, u32 y, const char *s);
u32 frequency_draw(u32 x, u32 y, u32 measure, double d);
void level_label_dbmv(u32 x, u32 y, u32 c, double v);
void voltage_uv_draw(u32 x, u32 y, u32 sg, double d);

void levels_max_panel_draw(void)
{
    u32 x, y, w;
    if (B8(0x42E68BB0u + 0x17))
        return;
    x = W32(BLK + 0xB0) + 8;
    y = W32(BLK + 0xB4) + 8;
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E64F58u);
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0xDDDDDD);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)0x400902C0u);
    x += H16(ADV);
    frequency_draw(x & 0xFFFF, y & 0xFFFF, 0, (double)(int)W32(BLK + 8));
    w = frequency_draw(x & 0xFFFF, y & 0xFFFF, 1, (double)(int)W32(BLK + 8));
    x = w + x + 0x14;
    level_label_dbmv(x & 0xFFFF, y & 0xFFFF, 1, D64(BLK + 0x10));
    voltage_uv_draw((H16(ADV) + x + 0x3C) & 0xFFFF, y & 0xFFFF, 0, D64(BLK + 0x18) * 1000.0);
}
""", src="ui")

wr(0x40060BA4, 448, "fft_spur_capture",
   "Records the spurs of the spectrum 0x42D364B4 (float) for the mode word 0x42E68668+0xF4 (0..3; the stock leaves its pointers unset for other values): with half = (s32 +0xF8) / 2, "
   "mean = sum of bins 2 .. half - 1 / (half - 2) is stored, then every bin i < half with (s64)(v x 10000) > (s64)(mean x 3 x 10000) appends i to the u16 index list and v x 2 to the value list, and the u16 count is stored. "
   "Lists per mode at 0x42E3C000: values +0x2340 / +0x1D00 / +0x12D8 / +0x18, indices +0x37F4 / +0x34D4 / +0x2FC0 / +0x2660, mean +0x14 / +0x10 / +0xC / +8, count +6 / +4 / +2 / +0.",
   COM + SPUR + """
void fft_spur_capture(void)
{
    u32 vals, idxs, mean, cntp, i, cnt;
    int half;
    double sum = 0.0;
    float v;
    long long a;
    spur_lists(W32(BLK + 0xF4), &vals, &idxs, &mean, &cntp);
    half = (int)W32(BLK + 0xF8) / 2;
    if ((u32)half >= 3)
        for (i = 2; i < (u32)half; i++)
            sum = (double)F32(SPEC + 4 * i) + sum;
    F32(mean) = (float)(sum / (double)(half - 2));
    cnt = 0;
    for (i = 0; i < (u32)half; i++) {
        v = F32(SPEC + 4 * i);
        a = F2LZ(v * 10000.0f);
        if (F2LZ(F32(mean) * 3.0f * 10000.0f) < a) {
            H16(cntp) = (unsigned short)cnt;
            H16(idxs + 2 * (cnt & 0xFFFF)) = (unsigned short)i;
            cnt = H16(cntp);
            F32(vals + 4 * cnt) = v * 2.0f;
            cnt++;
        }
    }
    H16(cntp) = (unsigned short)cnt;
}
""")

wr(0x40060DC8, 544, "fft_spur_cancel",
   "Unless touch byte 0x42E68BB0+0x17 is set: Hann window of the n = 0x42E68668+0xF8 floats at 0x42D164B4 into complex 0x42BEFE74 (as fft_hann_spectrum), byte +4 = 1, fft_radix2(n), cvec_magnitude into 0x42D364B4 (n / 2 bins). "
   "Then, unless byte +0x190 is set, each spur recorded by fft_spur_capture for the mode +0xF4 is subtracted from its bin, and a bin that falls below the stored mean ((s64) x 10000 compare) is set to the mean.",
   COM + SPUR + """
#define HIN 0x42D164B4u
#define HX 0x42BEFE74u
typedef double (*dfn)(double);
#define COS_T ((dfn)0x40066531u)   /* cos at 0x40066530 is Thumb */

void fft_radix2(int n, volatile float *x);
void cvec_magnitude(const volatile float *src, volatile float *dst, int n);

void fft_spur_cancel(void)
{
    u32 i, lim, vals, idxs, mean, cntp, k, cnt, idx;
    double dn, v;
    float s, m;
    long long a;
    if (B8(0x42E68BB0u + 0x17))
        return;
    i = 0;
    if (W32(BLK + 0xF8) != 0) {
        dn = (double)(int)(W32(BLK + 0xF8) - 1);
        do {
            v = (double)F32(HIN + 4 * i);
            lim = W32(BLK + 0xF8);
            F32(HX + 8 * i) = (float)(v * (0.5 - COS_T((double)i * 6.283185307179586 / dn) * 0.5));
            F32(HX + 8 * i + 4) = 0.0f;
            i++;
        } while (i < lim);
    }
    B8(BLK + 4) = 1;
    fft_radix2((int)W32(BLK + 0xF8), (volatile float *)HX);
    cvec_magnitude((volatile float *)HX, (volatile float *)SPEC, (int)W32(BLK + 0xF8) / 2);
    if (B8(BLK + 0x190))
        return;
    spur_lists(W32(BLK + 0xF4), &vals, &idxs, &mean, &cntp);
    cnt = H16(cntp);
    for (k = 0; k < cnt; k++) {
        idx = H16(idxs + 2 * k);
        s = F32(SPEC + 4 * idx) - F32(vals + 4 * k);
        F32(SPEC + 4 * idx) = s;
        m = F32(mean);
        a = F2LZ(m * 10000.0f);
        if (F2LZ(s * 10000.0f) < a)
            F32(SPEC + 4 * idx) = m;
    }
}
""")
