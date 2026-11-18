# Acquisition post-processing: the per-channel amplitude normalization, the two trigger-crossing
# searches that give the horizontal trigger offset, the Hann window feeding the FFT view and the
# radix-2 FFT itself.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def ww(addr, size, name, desc, body, src="acquisition"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define U64(a) (*(volatile unsigned long long *)(a))
"""

FFT = """
typedef struct { float re, im; } cfloat;
typedef double (*dfn)(double);
#define COS_T ((dfn)0x40066531u)   /* cos at 0x40066530 is Thumb */
#define SIN_T ((dfn)0x40066535u)   /* sin at 0x40066534 is Thumb */
"""

ww(0x4003FE14, 464, "final_per_channel_normalization",
   "Only when the u64 (ch 0: SYS+0xB78, else SYS+0xBA0) is at least (0x8D7F250 / SYS+0x524) x byte SYS+0x1B1. Works on the u16 trace SYS+0x7B4[ch] from index 10 to SYS+0x7BC - 10, split into half-periods at each crossing of the middle level m (SYS+0xB6C / +0xB94); "
   "the top / bottom references are SYS+0xB60 / +0xB64 (ch 1: +0xB88 / +0xB8C). For each half-period above m (mean >= m) whose peak exceeds m by more than (top - m) / 3, the samples are stretched so that the peak reaches top: v = m + (v - m) x (top - m) / (peak - m); "
   "below m the same with the bottom reference. The last sample of each half-period is not rescaled (stock).", COM + """
void final_per_channel_normalization(u32 ch)
{
    u32 buf = W32(SYS + 0x7B4 + 4 * ch), top, bot, end, sum, i, st, k;
    int mid, a, b, a3, b3, mx, mn, v, up, dn, cnt, avg;
    unsigned long long d, lim;
    if (ch == 0) {
        top = W32(SYS + 0xB60);
        bot = W32(SYS + 0xB64);
        mid = (int)W32(SYS + 0xB6C);
        d = (unsigned long long)W32(SYS + 0xB78) | ((unsigned long long)W32(SYS + 0xB7C) << 32);
    } else {
        top = W32(SYS + 0xB88);
        bot = W32(SYS + 0xB8C);
        mid = (int)W32(SYS + 0xB94);
        d = U64(SYS + 0xBA0);
    }
    lim = (unsigned long long)(0x8D7F250u / W32(SYS + 0x524)) * B8(SYS + 0x1B1);
    if (d < lim)
        return;
    end = W32(SYS + 0x7BC) - 10;
    a = (int)(top - (u32)mid);
    b = (int)((u32)mid - bot);
    if ((int)end <= 10)
        return;
    b3 = b / 3;
    a3 = a / 3;
    sum = 0;
    mx = 0;
    mn = 0xFFFFFF;
    st = 10;
    for (i = 10; (int)i < (int)end; i++) {
        v = H16(buf + 2 * i);
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
        sum += (u32)v;
        if (v < mid) {
            if ((int)H16(buf + 2 * i + 2) >= mid)
                goto seg;
        } else if (mid < v) {
            if (mid >= (int)H16(buf + 2 * i + 2))
                goto seg;
        }
        if (i + 1 != end)
            continue;
    seg:
        up = (int)((u32)mx - (u32)mid);
        dn = (int)((u32)mid - (u32)mn);
        cnt = (int)(i - st);
        avg = (int)sum / (cnt + 1);
        if (avg < mid) {
            if (b3 < dn && (int)st < (int)i)
                for (k = st; cnt; cnt--, k++)
                    H16(buf + 2 * k) = (unsigned short)((u32)mid + (u32)((int)(((u32)H16(buf + 2 * k) - (u32)mid) * (u32)b) / dn));
        } else {
            if (a3 < up && (int)st < (int)i)
                for (k = st; cnt; cnt--, k++)
                    H16(buf + 2 * k) = (unsigned short)((u32)mid + (u32)((int)(((u32)H16(buf + 2 * k) - (u32)mid) * (u32)a) / up));
        }
        sum = 0;
        mx = 0;
        mn = 0xFFFFFF;
        st = i;
    }
}
""")

ww(0x40040114, 656, "moving_average_and_crossing_alignment_search",
   "Only when bytes SYS+0x274, +0x275 and +0x279 are all set. With the trigger channel c = byte SYS+0x27C, h = u16 SYS+0x282, s = SYS+0x558 x h x 3 and t = byte SYS+0x1B1 x SYS+0x62C x h: copies n = 2s + 2t + 1 samples of trace SYS+0x7B4[c] from index SYS+0x2A0 - t - s to 0x416431E8, "
   "smooths them with an s-point moving average (result stored s - 1 places back, when s != 0), then searches from the middle (n/2 - s/2) to the left and to the right for the first crossing of the level SYS+0x1C0 x (s16) SYS+0x27E (rising, or falling when byte SYS+0x27B is set). "
   "The distances are limited to SYS+0x298; u16 SYS+0x29C = the nearer one (negative for the left one), the right one when both are equal, 0 when none.", COM + """
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);

#define AVG 0x416431E8u

void moving_average_and_crossing_alignment_search(void)
{
    u32 ch = B8(SYS + 0x27C), buf = W32(SYS + 0x7B4 + 4 * ch), h, s, t, lvl, n, sum, l, lim, old;
    int mid, k, fl = 0, fr = 0, dl = 0, dr = 0, off = 0;
    if (!B8(SYS + 0x274) || !B8(SYS + 0x275) || !B8(SYS + 0x279))
        return;
    h = H16(SYS + 0x282);
    s = W32(SYS + 0x558) * h * 3;
    lvl = W32(SYS + 0x1C0) * (u32)(int)(short)H16(SYS + 0x27E);
    t = B8(SYS + 0x1B1) * (W32(SYS + 0x62C) * h);
    l = W32(SYS + 0x2A0) - t - s;
    n = 2 * s + 2 * t + 1;
    copy_u16((unsigned short *)AVG, (const unsigned short *)(buf + 2 * l), n);
    if (s != 0 && (int)n > 0) {
        sum = 0;
        for (l = 0; (int)l < (int)n; l++) {
            sum += H16(AVG + 2 * l);
            if ((int)l >= (int)(s - 1)) {
                old = H16(AVG + 2 * (l + 1 - s));
                H16(AVG + 2 * (l + 1 - s)) = (unsigned short)((int)sum / (int)s);
                sum -= old;
            }
        }
    }
    mid = (int)n / 2 - (int)s / 2;
    if (B8(SYS + 0x27B) == 0) {
        for (k = mid; k > 0; k--)
            if ((int)H16(AVG + 2 * k) < (int)lvl && (int)H16(AVG + 2 * k + 2) >= (int)lvl) {
                fl = 1;
                dl = mid - k;
                break;
            }
        for (k = mid; k < (int)n; k++)
            if ((int)H16(AVG + 2 * k) < (int)lvl && (int)H16(AVG + 2 * k + 2) >= (int)lvl) {
                fr = 1;
                dr = k - mid;
                break;
            }
    } else {
        for (k = mid; k > 0; k--)
            if ((int)lvl < (int)H16(AVG + 2 * k) && (int)lvl >= (int)H16(AVG + 2 * k + 2)) {
                fl = 1;
                dl = mid - k;
                break;
            }
        for (k = mid; k < (int)n; k++)
            if ((int)lvl < (int)H16(AVG + 2 * k) && (int)lvl >= (int)H16(AVG + 2 * k + 2)) {
                fr = 1;
                dr = k - mid;
                break;
            }
    }
    lim = W32(SYS + 0x298);
    if (lim < (u32)dl)
        dl = (int)lim;
    if (lim < (u32)dr)
        dr = (int)lim;
    if (fl & fr)
        off = dl >= dr ? dr : -dl;
    else if (fl)
        off = -dl;
    else if (fr)
        off = dr;
    H16(SYS + 0x29C) = (unsigned short)off;
}
""")

ww(0x400403AC, 564, "trigger_window_crossing",
   "Only when byte SYS+0x275 is set and ch is the trigger channel (byte SYS+0x27C). Around the centre c = (s64 SYS+0x570 / s64 SYS+0x590) + SYS+0x7BC / 2 of trace SYS+0x7B4[ch], within c +- 6 x SYS+0x55C / SYS+0x554, it searches left and right for the nearest sample k where the trace crosses the level SYS+0x1C0 x (s16) SYS+0x27E "
   "between k - 1 and k + 1 (rising: before < level <= after, or before = level < after; falling with byte SYS+0x27B set, mirrored). u16 SYS+0x29E = the offset of the nearer crossing from c (the right one on a tie). "
   "When only the right one is found, the left distance is the window edge; when none is found nothing is stored.", COM + """
#define T16(b, k) ((int)H16((b) + 2u * (u32)(k)))

static int crossing(u32 b, int k, int lvl, u32 edge)
{
    int a = T16(b, k - 1), c = T16(b, k + 1);
    if (edge == 0)
        return (a < lvl && c >= lvl) || (a == lvl && c > lvl);
    return (a > lvl && c <= lvl) || (a == lvl && c < lvl);
}

void trigger_window_crossing(u32 ch)
{
    int lvl = (int)(W32(SYS + 0x1C0) * (u32)(int)(short)H16(SYS + 0x27E));
    u32 buf = W32(SYS + 0x7B4 + 4 * ch), w, edge;
    int found = 0, c, lo, hi, l, r, k, v;
    long long q;
    if (!B8(SYS + 0x275))
        return;
    if (ch != B8(SYS + 0x27C))
        return;
    q = S64(SYS + 0x570) / S64(SYS + 0x590);
    c = (int)((u32)q + (W32(SYS + 0x7BC) >> 1));
    w = (W32(SYS + 0x55C) * 6) / W32(SYS + 0x554);
    lo = (int)((u32)c - w);
    hi = (int)(w + (u32)c);
    edge = B8(SYS + 0x27B);
    l = lo;
    r = hi;
    for (k = c; lo < k; k--)
        if (crossing(buf, k, lvl, edge)) {
            found = 1;
            l = k;
            break;
        }
    for (k = c; k < hi; k++)
        if (crossing(buf, k, lvl, edge)) {
            r = k;
            goto done;
        }
    if (!found)
        return;
done:
    v = r - c;
    if (c - l < v)
        v = l - c;
    H16(SYS + 0x29E) = (unsigned short)v;
}
""")

ww(0x400665A8, 592, "fft_radix2",
   "In-place radix-2 decimation-in-time FFT of n complex floats (n a power of two, at least 2; the stock loops never end for n < 2 or other sizes). Bit-reversal reordering, a D-cache clean, then log2(n) stages with the twiddle w = cos(pi / half) - i sin(pi / half) advanced by cmul. "
   "After each stage, when byte SYS+0x54E is set and byte 0x42E68668+5 is clear, it calls runtime_event_service and stops (clearing byte 0x42E68668+4) when byte 0x42E68BB0+0x15 became set.", COM + FFT + """
cfloat cmul(cfloat a, cfloat b);
void dcache_clean_inv_all(void);
void runtime_event_service(void);

void fft_radix2(int n, volatile float *x)
{
    int i, j, k, t, stages, s, le, le2, jj, p;
    float r0, i0, r1, i1, tr, ti;
    cfloat u, w;
    double a;
    j = 0;
    for (i = 0; i < n - 1; i++) {
        if (i < j) {
            r0 = x[2 * i];
            i0 = x[2 * i + 1];
            r1 = x[2 * j];
            i1 = x[2 * j + 1];
            x[2 * j] = r0;
            x[2 * j + 1] = i0;
            x[2 * i] = r1;
            x[2 * i + 1] = i1;
        }
        k = n / 2;
        while (j >= k) {
            j -= k;
            k /= 2;
        }
        j += k;
    }
    dcache_clean_inv_all();
    stages = 1;
    t = n;
    for (;;) {
        t /= 2;
        if (t == 1)
            break;
        stages++;
    }
    for (s = 1; s <= stages; s++) {
        le = 2 << (s - 1);
        le2 = le / 2;
        u.re = 1.0f;
        u.im = 0.0f;
        a = 3.141592653589793 / (double)le2;
        w.re = (float)COS_T(a);
        w.im = (float)-SIN_T(a);
        for (jj = 0; jj <= le2 - 1; jj++) {
            for (i = jj; i <= n - 1; i += le) {
                p = i + le2;
                r1 = x[2 * p];
                i1 = x[2 * p + 1];
                tr = r1 * u.re - i1 * u.im;
                ti = r1 * u.im + i1 * u.re;
                x[2 * p] = x[2 * i] - tr;
                x[2 * p + 1] = x[2 * i + 1] - ti;
                x[2 * i] = x[2 * i] + tr;
                x[2 * i + 1] = x[2 * i + 1] + ti;
            }
            u = cmul(u, w);
        }
        if (B8(SYS + 0x54E) && !B8(0x42E68668u + 5)) {
            runtime_event_service();
            if (B8(0x42E68BB0u + 0x15)) {
                B8(0x42E68668u + 4) = 0;
                return;
            }
        }
    }
}
""")

ww(0x40041BA0, 408, "fft_hann_spectrum",
   "For each channel that is on (CH1: bytes SYS+0x3E and +0x42; CH2: +0x102 and +0x106) with n = SYS+0x7F8: x[i] = in[i] x (0.5 - 0.5 cos(2 pi i / (n - 1))) as complex floats with a zero imaginary part (CH1: in 0x42E49EB4, x 0x42E2DB08; CH2: in 0x42E4DEB4, x 0x42E31B08), "
   "then fft_radix2(n, x) and cvec_magnitude(x, out, n / 2) with out 0x42E4BEB4 / 0x42E4FEB4. The FFT is called even when n is 0.", COM + FFT + """
void fft_radix2(int n, volatile float *x);
void cvec_magnitude(const volatile float *src, volatile float *dst, int n);

static void hann_one(u32 in, u32 x, u32 out)
{
    volatile float *src = (volatile float *)in, *dst = (volatile float *)x;
    u32 n = W32(SYS + 0x7F8), i;
    double dn, v;
    if (n != 0) {
        dn = (double)(n - 1);
        i = 0;
        do {
            v = (double)src[i];
            dst[2 * i] = (float)(v * (0.5 - COS_T((double)i * 6.283185307179586 / dn) * 0.5));
            dst[2 * i + 1] = 0.0f;
            i++;
        } while (i < n);
    }
    fft_radix2((int)n, dst);
    cvec_magnitude(dst, (volatile float *)out, (int)(W32(SYS + 0x7F8) >> 1));
}

void fft_hann_spectrum(void)
{
    if (B8(SYS + 0x3E) && B8(SYS + 0x42))
        hann_one(0x42E49EB4u, 0x42E2DB08u, 0x42E4BEB4u);
    if (B8(SYS + 0x102) && B8(SYS + 0x106))
        hann_one(0x42E4DEB4u, 0x42E31B08u, 0x42E4FEB4u);
}
""")
