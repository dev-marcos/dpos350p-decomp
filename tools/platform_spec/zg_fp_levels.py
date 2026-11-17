# Floating-point functions of the level / measurement block 0x42E68668: marker values in dB and
# linear units, the dB conversion of the FFT magnitudes and the shaded level drawing.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wg(addr, size, name, desc, body, src="trace"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


MBG = '''#define MI(o) (*(volatile int *)(0x42E68668u + (o)))
#define MD(o) (*(volatile double *)(0x42E68668u + (o)))
#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))
'''

wg(0x4006053C, 328, "marker_values_compute",
   "Block 0x42E68668: the two marker positions (words +0x5C, +0x60 out of n = +0xB8 points) as s64 values lo + pos / n * span (lo = +0x120, span = +0x138) into +0x68 / +0x70 and their distance into +0x78; their levels top - y / scale * step (top +0x148, step +0x140, scale +0xCC, y from the u16 levels at 0x42E5F010) into +0x80 / +0x88 and the distance into +0x90; then 1000 * 10^(level / 20) into +0x98 / +0xA0 and the distance into +0xA8.", MBG + """
double pow(double x, double y);

void marker_values_compute(void)
{
    double span = (double)M64(0x138), n = (double)MI(0xB8), lo = (double)M64(0x120);
    double sc, top, stp, y0, y1, l0, l1, p0, p1;
    long long a, b;
    unsigned long long d;
    a = (long long)((double)MI(0x5C) / n * span + lo);
    b = (long long)((double)MI(0x60) / n * span + lo);
    d = (unsigned long long)b - (unsigned long long)a;
    if ((long long)d < 0)
        d = 0 - d;                   /* llabs */
    M64(0x70) = b;
    M64(0x68) = a;
    M64(0x78) = (long long)d;
    sc = (double)MI(0xCC);
    top = MD(0x148);
    stp = MD(0x140);
    y1 = (double)*(volatile short *)(0x42E5F010u + 2u * (u32)MI(0x60));
    y0 = (double)*(volatile short *)(0x42E5F010u + 2u * (u32)MI(0x5C));
    l0 = top - y0 / sc * stp;
    l1 = top - y1 / sc * stp;
    MD(0x88) = l1;
    MD(0x90) = __builtin_fabs(l1 - l0);
    MD(0x80) = l0;
    p0 = pow(10.0, MD(0x80) / 20.0) * 1000.0;
    p1 = pow(10.0, MD(0x88) / 20.0) * 1000.0;
    MD(0x98) = p0;
    MD(0xA8) = __builtin_fabs(p1 - p0);
    MD(0xA0) = p1;
}
""")

wg(0x40061DB0, 300, "fft_levels_db",
   "Unless touch byte +0x17 is set: for the n = word +0xF8 FFT magnitudes (floats at 0x42D364B4, divided by n): 20 * log10(m * 6.916 / g * f / 1000 * 80 / g) (g = u16 SYS+0x20, f = u64 SYS+0x50), plus offset +0xD0 * step +0x140, minus top +0x148, times scale +0xCC, over step; as s64, clamped to -4095 .. 4095 and stored as s16 through the pointer 0x42E68CB0.", MBG + """
double log10(double x);

void fft_levels_db(void)
{
    int n, i;
    volatile float *m = (volatile float *)0x42D364B4u;
    volatile short *dst;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    n = MI(0xF8);
    if (n < 1)
        return;
    dst = *(volatile short *volatile *)0x42E68CB0u;
    for (i = 0; i < n; i++) {
        float v = m[i] / (float)n;
        double g = (double)SYS16(0x20);
        double fr = (double)*(volatile unsigned long long *)(0x40544E68u + 0x50);
        double t = (double)v * 6.916 / g * fr / 1000.0 * 80.0 / g;
        double r = log10(t) * 20.0 + (double)MI(0xD0) * MD(0x140);
        long long q = (long long)((r - MD(0x148)) * (double)MI(0xCC) / MD(0x140));
        if (q < -4095)
            q = -4095;
        else if (q >= 0x1000)
            q = 0xFFF;
        *dst++ = (short)q;
    }
}
""")

wg(0x40062B1C, 352, "levels_shade_draw",
   "Unless touch byte +0x17 is set: for each level y from the min +0x168 (at least 0) below top +0xC0 of block 0x42E68668: colour from the 271-entry table 0x42E56C30 at (top - y) * 270 / (top - min), dimmed to (top - y) * 238 / (top - mean +0x160) + 0x11 of 255 above the mean (full below it); then a pixel at (x + +0xB0, y + +0xB4) for each x < +0xB8 whose level at 0x42E5F010 is at most y.", MBG + """
void draw_colour_set(u32 c);
void pixel_put(u32 x, u32 y);

void levels_shade_draw(void)
{
    int top = MI(0xC0), y, x, d;
    double k1, k2;
    volatile short *lv;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    k1 = 238.0 / (double)(top - MI(0x160));
    k2 = 270.0 / (double)(top - MI(0x168));
    y = MI(0x168);
    if (y >= top)
        return;
    do {
        u32 col, a, rr, gg, bb;
        if (y < 0)
            y = 0;
        d = top - y;
        col = *(volatile u32 *)(0x42E56C30u + 4u * (u32)(int)((double)(d < 0 ? 0 : d) * k2));
        if (MI(0x160) < y)
            a = (u32)((double)d * k1) + 0x11;
        else
            a = 0xFF;
        rr = a * ((col >> 16) & 0xFF) / 255u;
        gg = a * ((col >> 8) & 0xFF) / 255u;
        bb = a * (col & 0xFF) / 255u;
        draw_colour_set((rr << 16) | (gg << 8) | bb);
        lv = (volatile short *)0x42E5F010u;
        for (x = 0; x < MI(0xB8); x++, lv++) {
            if (y >= *lv)
                pixel_put((unsigned short)(x + MI(0xB0)), (unsigned short)(y + MI(0xB4)));
        }
        y++;
    } while (y < MI(0xC0));
}
""", src="display")

wg(0x40041D38, 252, "fft_db_traces_scale",
   "For each enabled channel (SYS+0x3E/+0x42, SYS+0x102/+0x106) and each of the n = SYS+0x7F8 FFT values (floats at 0x42E4BEB4 / 0x42E4FEB4): v = (int)(float)(45 * log10(x)), clamped to 0 .. h (h = half the height SYS+0xA), stored as h - v (u16) at 0x42E5B010 / 0x42E5C010 for fft_traces_draw.", """
double log10(double x);

static void scale(volatile float *s, volatile unsigned short *d, u32 n, int h)
{
    for (; n; n--) {
        int v = (int)(float)(log10((double)*s++) * 45.0);
        if (h < v)
            v = h;
        else if (v < 0)
            v = 0;
        *d++ = h - v;
    }
}

void fft_db_traces_scale(void)
{
    int h = SYS16(0xA) >> 1;
    if (SYS8(0x3E) && SYS8(0x42) && SYS32(0x7F8))
        scale((volatile float *)0x42E4BEB4u, (volatile unsigned short *)0x42E5B010u, SYS32(0x7F8), h);
    if (SYS8(0x102) && SYS8(0x106) && SYS32(0x7F8))
        scale((volatile float *)0x42E4FEB4u, (volatile unsigned short *)0x42E5C010u, SYS32(0x7F8), h);
}
""")

wg(0x40032B5C, 480, "channel_freq_measure",
   "Channel ch (0, or 1 and up; 0 / 1 need SYS+0x3E / +0x102): counts the crossings of the threshold word (SYS+0xB6C / +0xB94) by the u16 samples (pointer SYS+0x7B4 + 4ch) over the middle half of SYS+0x2B0 .. SYS+0x2B4. With at least two crossings spanning at least width * SYS+0x558 / 3 samples: samples per period (float, SYS+0xB70 / +0xB98), the period 2 * span * u64 SYS+0x590 / SYS+0x1C * SYS+0x55C / SYS+0x554 / (crossings - 1) (u64, SYS+0xB80 / +0xBA8) and 10^12 / period (SYS+0xB78 / +0xBA0). Otherwise 1000.0f, 10^12 and 0.", """
#define SB 0x40544E68u

void channel_freq_measure(u32 ch)
{
    volatile unsigned short *p = *(volatile unsigned short *volatile *)(SB + 0x7B4 + 4 * ch);
    volatile u32 *fo;
    volatile unsigned long long *o1, *o2;
    int thr, cnt = 0, first = 0, k, end;
    u32 f0 = 0, f1 = 0, e;
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
        thr = (int)SYS32(0xB6C);
        fo = (volatile u32 *)(SB + 0xB70);
        o2 = (volatile unsigned long long *)(SB + 0xB78);
        o1 = (volatile unsigned long long *)(SB + 0xB80);
    } else {
        if (ch == 1 && SYS8(0x102) == 0)
            return;
        thr = (int)SYS32(0xB94);
        fo = (volatile u32 *)(SB + 0xB98);
        o2 = (volatile unsigned long long *)(SB + 0xBA0);
        o1 = (volatile unsigned long long *)(SB + 0xBA8);
    }
    e = SYS32(0x2B4);
    k = (int)((e - SYS32(0x2B0)) >> 1);
    end = (int)(e - (u32)k);
    if (k < end) {
        volatile unsigned short *q = p + k;
        for (; k < end; k++, q++) {
            int a = q[0], hit;
            if (a < thr)
                hit = q[1] >= thr;
            else
                hit = thr < a && thr >= q[1];
            if (hit) {
                cnt++;
                if (first == 0) {
                    first = 1;
                    f0 = (u32)k;
                } else {
                    f1 = (u32)k;
                }
            }
        }
        if (cnt >= 2) {
            u32 diff = f1 - f0;
            if (diff >= (u32)(SYS16(8) * SYS32(0x558)) / 3u) {
                int c = cnt - 1;
                unsigned long long t;
                union { float f; u32 u; } fu;
                fu.f = (float)(int)diff / (float)c;
                *fo = fu.u;
                t = (unsigned long long)(long long)(int)diff * *(volatile unsigned long long *)(SB + 0x590);
                t = t / SYS16(0x1C);
                t = t * SYS32(0x55C);
                t = t / SYS32(0x554);
                t = t / (unsigned long long)(long long)c;
                *o1 = t * 2u;
                *o2 = 1000000000000ull / *o1;
                return;
            }
        }
    }
    *fo = 0x447A0000u;               /* 1000.0f */
    *o1 = 1000000000000ull;
    *o2 = 0;
}
""")
