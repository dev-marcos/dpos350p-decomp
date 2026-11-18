# Horizontal processing of a trace: the visible bounds, the shift of the stored samples when the
# horizontal position changes, the per-segment zoom and the gather of the segments into the
# display buffer. Mode 0/1 uses the first parameter set at SYS+0x5F4, other modes the second.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wv(addr, size, name, desc, body, src="trace"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
u32 mode_not_0_or_1(u32 v);
#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define I32(a) (*(volatile int *)(a))
#define S64(a) (*(volatile long long *)(a))
#define P(o) (SYS + 0x5F4 + (o))
#define TRBUF(ch) (0x411AF3E8u + (ch) * 0x927C0u)
"""

wv(0x4003BB6C, 300, "trace_view_bounds",
   "For modes other than 0 and 1: view_bounds_update. Otherwise the visible sample range of the screen: lo = W(SYS+0x608) + u16 SYS+0xC clamped to 0..width (u16 SYS+8), hi = W(SYS+0x60C) + u16 +0xC - 1 clamped to 0..width - 1 (0 when the sum is below 1), hi raised to lo; "
   "when the record (SYS+0x5D4) is shorter than SYS+0x5F8, the range is also cut to the centred part width x SYS+0x5D4 / SYS+0x5F8 of the screen. Results: u16 SYS+0x10 = lo, +0x12 = hi, +0x14 = hi - lo.", COM + """
void view_bounds_update(void);

void trace_view_bounds(u32 tr)
{
    u32 c, w, lo, hi, top, a, b;
    long long t;
    int q, half, qh, l1, h1;
    if (mode_not_0_or_1(tr)) {
        view_bounds_update();
        return;
    }
    c = H16(SYS + 0xC);
    t = (long long)I32(SYS + 0x608) + c;
    if (t < 0)
        lo = 0;
    else {
        lo = H16(SYS + 8);
        if (t < (long long)lo)
            lo = (u32)t;
    }
    H16(SYS + 0x10) = (unsigned short)lo;
    t = (long long)I32(SYS + 0x60C) + c;
    if (t < 1)
        hi = 0;
    else {
        w = H16(SYS + 8);
        if (t < (long long)w - 1)
            hi = (u32)t + 0xFFFF;
        else
            hi = w + 0xFFFF;
    }
    a = W32(SYS + 0x5F8);
    b = W32(SYS + 0x5D4);
    top = (unsigned short)lo;
    if (top < (unsigned short)hi)
        top = (unsigned short)hi;
    hi = top - lo;
    if (b < a) {
        w = H16(SYS + 8);
        q = (int)((long long)((unsigned long long)w * b) / (long long)a);
        half = (int)(w / 2);
        qh = q / 2;
        l1 = half - qh + 1;
        h1 = qh + half - 1;
        if (l1 > (int)(unsigned short)lo)
            lo = (u32)l1;
        if (h1 < (int)top)
            top = (u32)h1;
        hi = top - lo;
    }
    H16(SYS + 0x14) = (unsigned short)hi;
    H16(SYS + 0x12) = (unsigned short)top;
    H16(SYS + 0x10) = (unsigned short)lo;
}
""")

wv(0x4003C158, 428, "trace_shift_samples",
   "When the horizontal position changed: q = (SYS+0x5E0 - base) x scale / div / step (64-bit signed; mode 0/1: base SYS+0x570, scale +0x600, div SYS+0x590, step +0x604; other modes: base SYS+0x578, scale +0x618, div SYS+0x5A0, step +0x61C), "
   "the view offsets (+0x608 / +0x60C, or +0x620 / +0x624) are moved back by q and the stored samples of trace tr (buffer 0x411AF3E8 + tr x 0x927C0) are shifted by s = q x the samples per step (+0x5F8 or +0x610): "
   "for each of the u16 SYS+0x5F4 segments (start SYS+0x2C4[k], length SYS+0x454[k]) the samples move down by s (s > 0) or up by -s (s < 0), keeping the segment.", COM + """
void trace_shift_samples(u32 tr)
{
    long long d = S64(SYS + 0x5E0), q, s;
    volatile unsigned short *buf, *src, *dst;
    u32 n, k, st, ln;
    int cnt;
    if (!mode_not_0_or_1(tr)) {
        q = (long long)((unsigned long long)(d - S64(SYS + 0x570)) * W32(P(0xC))) / S64(SYS + 0x590);
        q = q / (long long)W32(P(0x10));
        W32(P(0x14)) -= (u32)q;
        W32(P(0x18)) -= (u32)q;
        s = (long long)((unsigned long long)q * W32(P(4)));
    } else {
        q = (long long)((unsigned long long)(d - S64(SYS + 0x578)) * W32(P(0x24))) / S64(SYS + 0x5A0);
        q = q / (long long)W32(P(0x28));
        W32(P(0x2C)) -= (u32)q;
        W32(P(0x30)) -= (u32)q;
        s = (long long)((unsigned long long)q * W32(P(0x1C)));
    }
    if (s == 0)
        return;
    n = H16(P(0));
    if (n == 0)
        return;
    buf = (volatile unsigned short *)TRBUF(tr);
    for (k = 0; k < n; k++) {
        st = W32(SYS + 0x2C4 + 4 * k);
        ln = W32(SYS + 0x454 + 4 * k);
        if (s > 0) {
            cnt = (int)(ln - (u32)s);
            if (cnt > 0) {
                dst = buf + st;
                src = buf + (st + (u32)s);
                do
                    *dst++ = *src++;
                while (--cnt);
            }
        } else {
            cnt = (int)((u32)s + ln);
            if (cnt > 0) {
                dst = buf + (ln + st);
                src = buf + ((u32)s + ln + st);
                do
                    *dst-- = *src--;
                while (--cnt);
            }
        }
    }
}
""")

wv(0x4003C314, 524, "trace_zoom_segments",
   "When the zoom factor k = SYS+0x554 is below the samples per step (mode 0/1: +0x5FC, else +0x614): rem = ((base - SYS+0x5E0) x per-sample (+0x5F8 / +0x610, signed) / div) %% step (bases and divisors as in trace_shift_samples), "
   "the view offsets become rem + offset x samples / k, and each segment k of trace tr is stretched with index_map_if_above(buffer, SYS+0x38C[k], u16 SYS+0x454[k], samples, rem as a 64-bit stack argument). mode_not_0_or_2(tr) is called and its result ignored (stock).", COM + """
u32 mode_not_0_or_2(u32 v);
void index_map_if_above(u32 a, u32 b, u32 c, u32 d, u32 e, u32 g);

void trace_zoom_segments(u32 tr)
{
    u32 k, wn, n, i, buf;
    int rem;
    long long x;
    u32 m = mode_not_0_or_1(tr);
    mode_not_0_or_2(tr & 0xFF);
    k = W32(SYS + 0x554);
    if (m == 0) {
        wn = W32(P(8));
        if (k >= wn)
            return;
        x = (long long)((unsigned long long)(S64(SYS + 0x570) - S64(SYS + 0x5E0)) * (unsigned long long)(long long)I32(P(4)));
        x = x / S64(SYS + 0x590);
        rem = (int)(x % (long long)W32(P(0x10)));
        {
            int a = (int)((long long)((unsigned long long)(long long)I32(P(0x18)) * W32(P(8))) / (long long)k);
            int b = (int)((long long)((unsigned long long)(long long)I32(P(0x14)) * W32(P(8))) / (long long)k);
            W32(P(0x18)) = (u32)(rem + a);
            W32(P(0x14)) = (u32)(rem + b);
        }
    } else {
        wn = W32(P(0x20));
        if (k >= wn)
            return;
        x = (long long)((unsigned long long)(S64(SYS + 0x578) - S64(SYS + 0x5E0)) * (unsigned long long)(long long)I32(P(0x1C)));
        x = x / S64(SYS + 0x5A0);
        rem = (int)(x % (long long)W32(P(0x28)));
        {
            int a = (int)((long long)((unsigned long long)(long long)I32(P(0x30)) * W32(P(0x20))) / (long long)k);
            int b = (int)((long long)((unsigned long long)(long long)I32(P(0x2C)) * W32(P(0x20))) / (long long)k);
            W32(P(0x30)) = (u32)(rem + a);
            W32(P(0x2C)) = (u32)(rem + b);
        }
    }
    buf = TRBUF(tr & 0xFF);
    n = H16(P(0));
    for (i = 0; (int)i < (int)n; i = i + 1, n = H16(P(0)))
        index_map_if_above(buf, W32(SYS + 0x38C + 4 * i), (unsigned short)W32(SYS + 0x454 + 4 * i), wn, (u32)rem,
                           (u32)(rem >> 31));
}
""")

wv(0x4003C5CC, 452, "trace_segments_gather",
   "Builds the display samples of trace tr for screen columns start .. start + n: with z = the samples per column (mode 0/1: +0x5F8, else +0x610), c = 2z + 1 values per segment and column, "
   "and per segment k the offset o[k] = SYS+0x38C[k] - SYS+0x2C4[k] - z x width / 2: dest[(col x segs + k) x c + j] = src[k x SYS+0x2B0 + z x col + j + o[k]] for j < c, the source index clamped to 0..0x493E0 "
   "unless z equals the record setting SYS+0x5D4 (then no offset, no clamp). src = 0x411AF3E8 + tr x 0x927C0, dest = 0x413F92E8 + tr x 0x927C0. At most 101 segments (stock stack array).", COM + """
void trace_segments_gather(u32 tr, u32 start, u32 n)
{
    int off[101];
    u32 segs = H16(P(0)), rec = W32(SYS + 0x2B0), rl = W32(SYS + 0x5D4), end = start + n + 1, z, c;
    volatile unsigned short *src, *dst;
    int i, k, j, half, si;
    z = mode_not_0_or_1(tr) ? W32(P(0x1C)) : W32(P(4));
    c = 1 + 2 * z;
    if ((int)segs > 0) {
        half = (int)(z * H16(SYS + 8));
        half /= 2;
        for (k = 0; k < (int)segs; k++)
            off[k] = (int)(W32(SYS + 0x38C + 4 * k) - W32(SYS + 0x2C4 + 4 * k)) - half;
    }
    src = (volatile unsigned short *)TRBUF(tr);
    dst = (volatile unsigned short *)(0x413F92E8u + tr * 0x927C0u);
    for (i = (int)start; i < (int)end; i++) {
        if ((int)segs <= 0)
            continue;
        for (k = 0; k < (int)segs; k++) {
            if ((int)c <= 0)
                continue;
            for (j = 0; j < (int)c; j++) {
                si = (int)((u32)k * rec + z * (u32)i + (u32)j);
                if (z != rl) {
                    si += off[k];
                    if (si < 0)
                        si = 0;
                    else if (si > 0x493E0)
                        si = 0x493E0;
                }
                dst[(u32)i * segs * c + (u32)k * c + (u32)j] = src[si];
            }
        }
    }
}
""")
