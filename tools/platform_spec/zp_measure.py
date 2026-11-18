# Automatic measurements of one channel: min / max / peak-to-peak / middle level of the raw
# samples, voltages, crossings of the trigger level, period, frequency, high and low times,
# duty cycle, RMS and mean, with a running average over SYS+0x86E / SYS+0x870 acquisitions.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wp(addr, size, name, desc, body, src="acquisition"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


wp(0x40032E0C, 4684, "measure_channel",
   "Measurements of channel ch (0 or 1; any other value runs as channel 1 without its enable check) from the sample buffer word SYS+0x7B4 + 4 * ch (u16 samples, SYS+0x558 interleaved banks, count u16 SYS+0x7BC). "
   "Raw block M (SYS+0xB28 / SYS+0xB44, words): max, min, max - min and middle, divided by SYS+0x1C0; +0x10 glitch count per bank; +0x18 hysteresis limit. "
   "Result block O (SYS+0x8E8 / SYS+0x948, 12 x s64): 0 max and 1 min voltage, 2 mean and 3 RMS (x 100 then rescaled), 4 the larger of |max| and |min| with its sign, 5 peak-to-peak, 6 frequency = 1e15 / period, 7 period, 8 and 9 the two pulse widths, 10 duty cycle and 11 its complement (x 10000). "
   "Voltage = (raw - offset word SYS+0x60 / SYS+0x120) * scale s64 SYS+0x50 / SYS+0x110 / u16 SYS+0x20. "
   "Crossings of the trigger level s16 SYS+0x27E (the middle level when SYS+0x27C is another channel or the level is outside min..max) give the period in samples x 1e6, converted with the time base s64 SYS+0x590 and u16 SYS+0x1C. "
   "O is added to the sums at SYS+0xA68 / SYS+0xAC8; after u16 SYS+0x86E / SYS+0x870 acquisitions (counter u16 SYS+0x86A / SYS+0x86C) the averages go to SYS+0x9A8 / SYS+0xA08, the sums are cleared and the byte SYS+0x872 / SYS+0x873 is set.", """
long long __aeabi_ldivmod(long long a, long long b);
double sqrt(double x);

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define S16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define B8(a) (*(volatile unsigned char *)(a))
#define Q64(a) (*(volatile long long *)(a))
#define LDIV(a, b) __aeabi_ldivmod((long long)(a), (long long)(b))
#define MUL64(a, b) ((long long)((unsigned long long)(a) * (unsigned long long)(b)))

/* Scale v down by 10 until it is at most 1e6, moving the factor into the divisor k (while k > 1)
   or the multiplier m (once k is 1). The first step always divides k (as in the stock code). */
static long long dec_first(long long v, long long *k, long long *m)
{
    v = LDIV(v, 10);
    *k = LDIV(*k, 10);
    while (v > 1000000) {
        v = LDIV(v, 10);
        if (*k == 1)
            *m = MUL64(*m, 10);
        else
            *k = LDIV(*k, 10);
    }
    return v;
}

static long long dec_more(long long v, long long *k, long long *m)
{
    do {
        v = LDIV(v, 10);
        if (*k == 1)
            *m = MUL64(*m, 10);
        else
            *k = LDIV(*k, 10);
    } while (v > 1000000);
    return v;
}

/* (v * timebase) / SYS+0x1C / banks / k * m, with both v and the time base first scaled down. */
static long long to_time(long long *v, long long *k, long long *m, int banks)
{
    long long tb, r;
    if (*v > 1000000)
        *v = dec_first(*v, k, m);
    tb = Q64(SYS + 0x590);
    if (tb > 1000000)
        tb = dec_more(tb, k, m);
    r = LDIV(MUL64(*v, tb), (long long)H16(SYS + 0x1C));
    r = LDIV(r, (long long)banks);
    r = LDIV(r, *k);
    return MUL64(r, *m);
}

void measure_channel(u32 ch_arg)
{
    static const unsigned char order[12] = {0, 1, 2, 3, 5, 4, 6, 7, 8, 9, 10, 11};
    unsigned char ch = (unsigned char)ch_arg;   /* the stock code reads the byte back from the stack */
    u32 th = (u32)(int)S16(SYS + 0x27E);
    u32 buf = W32(SYS + 0x7B4 + 4 * ch), n = (unsigned short)W32(SYS + 0x7BC);
    u32 cntp, flagp, avg, out, acc, mb, limit, mx, mn, d, maxs, mins, k;
    volatile u32 *mr;
    volatile long long *o, *a, *v;
    long long off, scale, vmax, vmin, amax, amin, per = 0, wb = 0, wa = 0, rms = 0, mean = 0;
    long long k1 = 1000000, m1 = 1, k2 = 1000000, m2 = 1, k3 = 1000000, m3 = 1, t, w, div;
    int banks, seg, b, glitch = 0, bad = 0, i;

    if (ch == 0) {
        if (B8(SYS + 0x3E) == 0)
            return;
        off = (long long)(int)W32(SYS + 0x60);
        scale = Q64(SYS + 0x50);
        flagp = SYS + 0x872;
        cntp = SYS + 0x86A;
        limit = H16(SYS + 0x86E);
        avg = SYS + 0x9A8;
        out = SYS + 0x8E8;
        acc = SYS + 0xA68;
        mb = SYS + 0xB28;
    } else {
        if (ch == 1 && B8(SYS + 0x102) == 0)
            return;
        off = (long long)(int)W32(SYS + 0x120);
        scale = Q64(SYS + 0x110);
        flagp = SYS + 0x873;
        cntp = SYS + 0x86C;
        limit = H16(SYS + 0x870);
        avg = SYS + 0xA08;
        out = SYS + 0x948;
        acc = SYS + 0xAC8;
        mb = SYS + 0xB44;
    }
    banks = (int)W32(SYS + 0x558);
    mr = (volatile u32 *)mb;
    o = (volatile long long *)out;
    a = (volatile long long *)acc;
    v = (volatile long long *)avg;

    /* raw min / max over the samples 10 .. n - 11 */
    mx = 0;
    mn = 0xFFFFFFFFu;
    if ((int)(n - 10) > 10) {
        volatile unsigned short *p = (volatile unsigned short *)(buf + 0x14);
        for (k = n - 20; k; k--, p++) {
            u32 s = *p;
            if (s >= mx)
                mx = s;
            if (s <= mn)
                mn = s;
        }
    }
    mr[1] = mn;
    mr[3] = (mn + mx) >> 1;
    mr[2] = mx - mn;
    mr[0] = mx;
    d = W32(SYS + 0x1C0);
    maxs = mx / d;
    mr[0] = maxs;
    {
        u32 lo = mr[1], mid = mr[3];
        mins = lo / W32(SYS + 0x1C0);
        mr[1] = mins;
        mr[2] = (mx - mn) / W32(SYS + 0x1C0);
        mr[3] = mid / W32(SYS + 0x1C0);
    }
    vmax = LDIV(MUL64((long long)maxs - off, scale), (long long)H16(SYS + 0x20));
    o[0] = vmax;
    vmin = LDIV(MUL64((long long)mins - off, scale), (long long)H16(SYS + 0x20));
    o[1] = vmin;
    o[5] = vmax - vmin;
    if (B8(SYS + 0x27C) != ch)
        th = mr[3];
    if (maxs < th || th < mins)
        th = mr[3];
    amax = vmax < 0 ? -vmax : vmax;
    amin = vmin < 0 ? -vmin : vmin;
    o[4] = amax < amin ? vmin : vmax;

    seg = (unsigned short)((int)(n - 20) / banks);
    for (b = 0; b < banks; b++) {
        u32 base = (u32)b + 10, shift = W32(SYS + 0x1BC);
        int hia = 0, loa = 0xFFFF, hib = 0, lob = 0xFFFF, fa = 0, fb = 0;
        int na = 0, nb = 0, firsta = 0, lasta = 0, firstb = 0, lastb = 0;
        int suma = 0, sumb = 0, cnta = 0, cntb = 0, usea = 0, from, to, cnt, sq, sm;
        int prev = (int)(H16(buf + 0x14 + 2 * (u32)b) >> shift);
        volatile unsigned short *p;
        if (seg <= 1)
            goto invalid;
        for (i = 1; i < seg; i++) {
            u32 s;
            if (prev >= hia)
                hia = prev;
            s = H16(buf + 2 * ((u32)banks * (u32)i + base)) >> shift;
            if (prev <= loa)
                loa = prev;
            if (prev >= hib)
                hib = prev;
            if (prev <= lob)
                lob = prev;
            if (prev < (int)th) {
                if ((int)th <= (int)(unsigned short)s) {
                    if (fa == 0)
                        firsta = i;
                    na++;
                    fa = 1;
                    lasta = i;
                    if (fb) {
                        cnta++;
                        suma = i + suma - lastb;
                    }
                    if (mr[6] < (u32)(hia - loa + 1)) {
                        loa = 0xFFFF;
                        glitch++;
                        hia = 0;
                    }
                    i += 2;
                }
            } else if ((int)th < prev && (int)th >= (int)(unsigned short)s) {
                if (fb == 0)
                    firstb = i;
                nb++;
                fb = 1;
                lastb = i;
                if (fa) {
                    cntb++;
                    sumb = i + sumb - lasta;
                }
                if (mr[6] < (u32)(hib - lob + 1)) {
                    lob = 0xFFFF;
                    glitch++;
                    hib = 0;
                }
                i += 2;
            }
            prev = (unsigned short)s;
        }
        if (na <= 1 && nb <= 1)
            goto invalid;
        if (na >= nb) {
            per += LDIV((long long)(lasta - firsta) * 1000000, (long long)(na - 1));
            wb += LDIV((long long)sumb * 1000000, (long long)cntb);
            wa = wb < per ? per - wb : 0;
            usea = 1;
        } else {
            per += LDIV((long long)(lastb - firstb) * 1000000, (long long)(nb - 1));
            wa += LDIV((long long)suma * 1000000, (long long)cnta);
            wb = wa < per ? per - wa : 0;
        }
        if (bad) {
            to = seg;
            from = 0;
        } else if (usea) {
            from = firsta;
            to = lasta;
        } else {
            to = lastb;
            from = firstb;
        }
        goto rms_part;
invalid:
        bad = 1;
        mr[4] = 0;
        to = seg;
        from = 0;
rms_part:
        sq = 0;
        sm = 0;
        p = (volatile unsigned short *)(buf + 2 * ((u32)banks * (u32)from + base));
        if (to >= from) {
            u32 sh = W32(SYS + 0x1BC);
            int j = from;
            do {
                int x = (int)(*p >> sh) - (int)(u32)off;
                j++;
                p += banks;
                sq += x * x;
                sm += x;
            } while (to >= j);
        }
        cnt = to - from + 1;
        rms = (long long)((double)rms + sqrt((double)(sq / cnt)) * 100.0);
        mean += (long long)(100 * sm / cnt);
    }
    mr[4] = (u32)(glitch / banks);
    o[3] = LDIV(LDIV(LDIV(MUL64(rms, scale), (long long)H16(SYS + 0x20)), 100), (long long)banks);
    o[2] = LDIV(LDIV(LDIV(MUL64(mean, scale), (long long)H16(SYS + 0x20)), 100), (long long)banks);

    t = to_time(&per, &k1, &m1, banks);
    o[7] = t;
    w = to_time(&wb, &k2, &m2, banks);
    o[8] = w;
    o[9] = to_time(&wa, &k3, &m3, banks);
    o[6] = t != 0 ? LDIV(1000000000000000LL, t) : 0;
    div = w > 10000000000LL ? 10000 : 1;
    {
        long long duty = LDIV(MUL64(LDIV(w, div), 10000), LDIV(t, div));
        o[11] = 10000 - duty;
        o[10] = duty;
    }
    a[0] += vmax;
    for (i = 1; i < 12; i++)
        a[order[i]] += o[order[i]];
    k = (unsigned short)(H16(cntp) + 1);
    if ((int)limit > (int)k) {
        H16(cntp) = (unsigned short)k;
        return;
    }
    H16(cntp) = (unsigned short)k;
    B8(flagp) = 1;
    for (i = 0; i < 12; i++)
        v[order[i]] = LDIV(a[order[i]], (long long)(int)limit);
    for (i = 0; i < 12; i++)
        a[i] = 0;
    H16(cntp) = 0;
}
""")

wp(0x400301E0, 2328, "value_si_scale",
   "Scales a measurement for display. kind is the measurement number (1..12, see measure_item_draw); v is in the base unit (uV for 1..6, mHz for 7, ps for 8..10, 1/10000 for 11..12). "
   "Kinds 1..6 truncate and kinds 7..10 round half up (rounding is skipped when v is negative); both pick, from |v|, the divisor, the number of decimals *dec and the prefix index *prefix (0..4 = u / m / unit / k for volts, mHz / Hz / kHz / MHz / GHz, ps / ns / us / ms / s). From |v| >= 1e14 nothing is changed. "
   "Other kinds set *dec to 2 (0 when v is 0) and leave *prefix. Finally up to four trailing zero digits are dropped while *dec > 0. *out gets the scaled value.", """
static long long q(long long v, long long d)
{
    return v / d;
}

/* rounded: add d when the remainder is at least the stock threshold (half + 1, 6 for d = 10) */
static long long qr(long long v, long long d, long long half)
{
    if (v % d >= half)
        v += d;
    return v / d;
}

void value_si_scale(u32 kind, u32 unused, long long v, long long *out, int *dec, int *prefix)
{
    long long a = v < 0 ? -v : v;
    int d, i;
    (void)unused;
    if (kind >= 1 && kind <= 6) {
        if (a < 1000) {
            *dec = 0; *prefix = 0;
        } else if (a < 10000) {
            v = q(v, 10); *dec = 2; *prefix = 1;
        } else if (a < 100000) {
            v = q(v, 100); *dec = 1; *prefix = 1;
        } else if (a < 1000000) {
            v = q(v, 1000); *dec = 0; *prefix = 1;
        } else if (a < 10000000) {
            v = q(v, 10000); *dec = 2; *prefix = 2;
        } else if (a < 100000000) {
            v = q(v, 100000); *dec = 1; *prefix = 2;
        } else if (a < 1000000000) {
            v = q(v, 1000000); *dec = 0; *prefix = 2;
        } else if (a < 10000000000LL) {
            v = q(v, 10000000); *dec = 2; *prefix = 3;
        } else if (a < 100000000000LL) {
            v = q(v, 100000000); *dec = 1; *prefix = 3;
        } else if (a < 1000000000000LL) {
            v = q(v, 1000000000); *dec = 0; *prefix = 3;
        } else if (a < 10000000000000LL) {
            v = q(v, 10000000000LL); *dec = 2; *prefix = 4;
        } else if (a < 100000000000000LL) {
            v = q(v, 100000000000LL); *dec = 1; *prefix = 4;
        }
    } else if (kind >= 7 && kind <= 10) {
        if (a < 1000) {
            *dec = 0; *prefix = 0;
        } else if (a < 10000) {
            *dec = 3; *prefix = 1;
        } else if (a < 100000) {
            v = qr(v, 10, 6); *dec = 2; *prefix = 1;
        } else if (a < 1000000) {
            v = qr(v, 100, 51); *dec = 1; *prefix = 1;
        } else if (a < 10000000) {
            v = qr(v, 1000, 501); *dec = 3; *prefix = 2;
        } else if (a < 100000000) {
            v = qr(v, 10000, 5001); *dec = 2; *prefix = 2;
        } else if (a < 1000000000) {
            v = qr(v, 100000, 50001); *dec = 1; *prefix = 2;
        } else if (a < 10000000000LL) {
            v = qr(v, 1000000, 500001); *dec = 3; *prefix = 3;
        } else if (a < 100000000000LL) {
            v = qr(v, 10000000, 5000001); *dec = 2; *prefix = 3;
        } else if (a < 1000000000000LL) {
            v = qr(v, 100000000, 50000001); *dec = 1; *prefix = 3;
        } else if (a < 10000000000000LL) {
            v = qr(v, 1000000000, 500000001); *dec = 3; *prefix = 4;
        } else if (a < 100000000000000LL) {
            v = qr(v, 10000000000LL, 5000000001LL); *dec = 2; *prefix = 4;
        }
    } else {
        *dec = v != 0 ? 2 : 0;
    }
    d = *dec;
    for (i = 0; i < 4; i++) {
        if (v % 10 == 0 && d > 0) {
            d--;
            v /= 10;
        }
    }
    *dec = d;
    *out = v;
}
""", src="display")

MI = """void half_42e68be2_set(u32 a);
void draw_colour_set(u32 c);
void word_42e68c74_set(u32 a);
void text_draw(u32 x, u32 y, const char *s);
void func_4002d82c_flag0(u32 x, u32 y, u32 lo, u32 hi, u32 dec);
void value_si_scale(u32 kind, u32 unused, long long v, long long *out, int *dec, int *prefix);
#define H16(a) (*(volatile unsigned short *)(a))
#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))
"""

wp(0x40030AF8, 1116, "measure_item_draw",
   "One measurement readout at (x, y) for channel ch (0 = SYS+0x9A8 averages, else SYS+0xA08): the label in grey 0x888888 (1 MAX, 2 MIN, 3 AVG, 4 RMS, 5 VP, 6 VPP, 7 F, 8 T, 9 T+, 10 T-, 11 Dt+, 12 Dt-), then in the channel colour (0xFFFF00 for ch 0, 0x00FFFF otherwise) a sign ('+' / '-' for MAX, MIN, AVG and VP, nothing for 0), the number from value_si_scale in the font 0x42E66718, and the unit (uV / mV / V / kV, mHz .. GHz, pS .. S, %). The x position follows u16 0x42E68BE4 after each text. Kinds outside 1..12 make the stock code use a value it never loads (the caller's r4:r5), so they are not covered. The decimals and prefix start at 0 here; the stock code leaves them uninitialised for values of 1e14 and above.", MI + """
void measure_item_draw(u32 x, u32 y, u32 ch, u32 kind)
{
    static const char *const label[12] = {"MAX", "MIN", "AVG", "RMS", "VP", "VPP", "F", "T", "T+", "T-", "Dt+", "Dt-"};
    static const char *const volt[5] = {"uV", "mV", "V", 0, "kV"};
    static const char *const hz[5] = {"mHz", "Hz", "KHz", "MHz", "GHz"};
    static const char *const sec[5] = {"pS", "nS", "uS", "mS", "S"};
    volatile long long *avg = (volatile long long *)(ch == 0 ? SYS + 0x9A8 : SYS + 0xA08);
    char sign[2];
    long long v = 0, out;
    int dec = 0, prefix = 0;
    u32 yy, x2, xn, xu;
    const char *unit = 0;
    sign[1] = 0;
    half_42e68be2_set(2);
    draw_colour_set(0x888888);
    x += 3;
    word_42e68c74_set(0x42E658D8u);
    y += 4;
    if (kind - 1 <= 11) {
        text_draw(U16(x), U16(y), label[kind - 1]);
        v = avg[kind - 1];
    }
    if (kind == 1 || kind == 2 || kind == 3 || kind == 5) {
        if (v == 0) {
            sign[0] = 0;
        } else if (v > 0) {
            sign[0] = '+';
        } else {
            sign[0] = '-';
            v = -v;
        }
    } else {
        sign[0] = 0;
        if (v < 0)
            v = -v;
    }
    value_si_scale(kind, 0, v, &out, &dec, &prefix);
    v = out;
    yy = y - 3;
    draw_colour_set(ch == 0 ? 0xFFFF00u : 0x00FFFFu);
    x2 = H16(0x42E68BE4u) + x + 3;
    text_draw(U16(x2), U16(yy), sign);
    half_42e68be2_set(2);
    xn = H16(0x42E68BE4u) + x2;
    word_42e68c74_set(0x42E66718u);
    func_4002d82c_flag0(U16(xn), U16(yy - 2), (u32)v, (u32)((unsigned long long)v >> 32), (unsigned char)dec);
    xu = H16(0x42E68BE4u) + xn + 3;
    half_42e68be2_set(1);
    draw_colour_set(0x888888);
    word_42e68c74_set(0x42E658D8u);
    if (kind >= 1 && kind <= 6) {
        if ((u32)prefix <= 4)
            unit = volt[prefix];
    } else if (kind == 7) {
        if ((u32)prefix <= 4)
            unit = hz[prefix];
    } else if (kind >= 8 && kind <= 10) {
        if ((u32)prefix <= 4)
            unit = sec[prefix];
    } else if (kind == 11 || kind == 12) {
        unit = "%";
    }
    if (unit)
        text_draw(U16(xu), U16(yy - 2 + 5), unit);
}
""", src="display")

wp(0x40030F8C, 400, "measure_panel_draw",
   "The measurement panel at the bottom of the screen, unless the touch byte 0x42E68BB0+0x17 or the byte SYS+0x29 is set. The list SYS+0x880 holds up to 24 s16 items (0 ends it): a positive item is that measurement of channel 0, a negative one of channel 1. Item i is in column i % 6 and row i / 6 (rows go up from y = 0x206): a 155 x 24 box in 0x44333333 (the last item, unless in column 5, gets a 149-pixel box with a slanted right edge), then measure_item_draw at (0xA2 * column + 0x10, 0x209 - 24 * row).", """
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void measure_item_draw(u32 x, u32 y, u32 ch, u32 kind);
#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

void measure_panel_draw(void)
{
    volatile short *list = (volatile short *)(SYS + 0x880);
    int last = 0x17, i, k;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    if (*(volatile unsigned char *)(SYS + 0x29))
        return;
    for (i = 0; i < 24; i++)
        if (list[i] == 0) {
            last = i - 1;
            break;
        }
    for (i = 0; i < 24; i++) {
        int v = list[i], ch = 0, row = i / 6, col = i - 6 * row;
        u32 x = 0x9B * col + 0xB, y = 0x206 - 24 * row;
        if (v == 0)
            return;
        if (v < 0) {
            ch = 1;
            v = -v;
        }
        draw_colour_set(0x44333333u);
        if (col != 5 && i == last) {
            u32 x1 = U16(x + 0x95), x2 = x1;
            rect_fill(U16(x), U16(y), 0x95, 0x18);
            for (k = 0; k < 24; k++, y++, x2++)
                func_4002ecc4_span(U16(y), x1, U16(x2));
        } else {
            rect_fill(U16(x), U16(y), 0x9B, 0x18);
        }
        measure_item_draw(U16(0xA2 * col + 0x10), U16(0x209 - 24 * row), (unsigned char)ch, (unsigned char)v);
    }
}
""", src="display")

wp(0x4003117C, 1424, "manual_cursors_draw",
   "The manual cursors, unless the touch byte 0x42E68BB0+0x17 or SYS+0x652 is set, or SYS+0x28 is set while SYS+0x3C is 0. Byte SYS+0x802 enables the two time cursors H1 / H2 (words SYS+0x808 / SYS+0x80C, x from the trace left edge), byte SYS+0x803 the two level cursors V1 / V2 (words SYS+0x810 / SYS+0x814, y from the top). "
   "Each cursor is a dotted grey 0x888888 line (dash 3, period 4); the H cursors get a 17 x 14 marker with a triangle and 'H1' / 'H2' at the top, the V cursors a triangle and 'V1' / 'V2' at the right edge (x = 0x39E .. 0x3B6), kept 8 pixels inside the trace area. With the zoom byte SYS+0x2AA set, the area starts 2 pixels right and a third of its height lower, and the V positions are scaled by 2/3.", """
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void text_draw(u32 x, u32 y, const char *s);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void vline_draw(u32 x, u32 y0, u32 y1);
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define B8(a) (*(volatile unsigned char *)(a))
#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

static void hmark(u32 xc, u32 top, const char *digit, unsigned short ty1, unsigned short ty2, u32 dx2)
{
    u32 y = top + 0xB, j;
    draw_colour_set(0xDDDDDD);
    rect_fill(U16(xc - 8), U16(top), 0x11, 0xE);
    word_42e68c74_set(0x42E65418u);
    for (j = 0; j <= 8; j++) {
        vline_draw(U16(xc - j), U16(y), U16(y + 0xD - j));
        vline_draw(U16(xc + j), U16(y), U16(y + 0xD - j));
    }
    draw_colour_set(0);
    text_draw(U16(xc - 3), ty1, "H");
    text_draw(U16(xc - dx2), ty2, digit);
}

static int clamp_y(int y)
{
    int lo = (int)H16(SYS + 6) + 8, hi = (int)(H16(SYS + 6) + H16(SYS + 0xA)) - 8;
    if (y < lo)
        return lo;
    return hi > y ? y : hi;
}

static void vmark(int pos)
{
    int j;
    draw_colour_set(0xDDDDDD);
    for (j = 0; j <= 8; j++) {
        func_4002ecc4_span(U16(pos - j), U16(0x39E + j), 0x3AA);
        func_4002ecc4_span(U16(pos + j), U16(0x39E + j), 0x3AA);
    }
    rect_fill(0x3AB, U16(pos - 8), 0xB, 0x11);
}

void manual_cursors_draw(void)
{
    u32 x0, y0, right, bottom, h, h1, h2, v1, v2, i;
    int nx, ny, p;
    if (B8(0x42E68BB0u + 0x17))
        return;
    if (B8(SYS + 0x652))
        return;
    if (B8(SYS + 0x28) && B8(SYS + 0x3C) == 0)
        return;
    if (B8(SYS + 0x802) == 0 && B8(SYS + 0x803) == 0)
        return;
    x0 = H16(SYS + 4);
    y0 = H16(SYS + 6);
    h = H16(SYS + 0xA);
    right = x0 + H16(SYS + 8);
    bottom = y0 + h;
    h1 = W32(SYS + 0x808);
    h2 = W32(SYS + 0x80C);
    v1 = W32(SYS + 0x810);
    v2 = W32(SYS + 0x814);
    if (B8(SYS + 0x2AA) == 0) {
        x0 += 1;
        y0 += 2;
    } else {
        x0 += 2;
        y0 = y0 + ((h * 0xAAABu) >> 17) + 3;
        v1 = (u32)((int)(v1 << 1) / 3);
        v2 = (u32)((int)(v2 << 1) / 3);
    }
    nx = (int)(right - x0 + 1) / 4;
    ny = (int)(bottom - y0 + 1) / 4;
    draw_colour_set(0x888888);
    if (B8(SYS + 0x802) && ny > 0) {
        u32 xa = U16(h1 + x0 + 0xFFFF), xb = h2 + x0 + 0xFFFF;
        for (i = 0; (int)i < ny; i++)
            vline_draw(xa, U16(y0 + 4 * i), U16(y0 + 4 * i + 2));
        for (i = 0; (int)i < ny; i++)
            vline_draw(U16(xb), U16(y0 + 4 * i), U16(y0 + 4 * i + 2));
    }
    if (B8(SYS + 0x803) && nx > 0) {
        u32 ya = U16(v1 + y0 + 0xFFFF), yb = v2 + y0 + 0xFFFF;
        for (i = 0; (int)i < nx; i++)
            func_4002ecc4_span(ya, U16(x0 + 4 * i), U16(x0 + 4 * i + 2));
        for (i = 0; (int)i < nx; i++)
            func_4002ecc4_span(U16(yb), U16(x0 + 4 * i), U16(x0 + 4 * i + 2));
    }
    if (B8(SYS + 0x802)) {
        u32 r7 = y0 - 1 + 0xB;
        hmark(h1 + H16(SYS + 4), y0 - 1, "1", U16(r7 - 0xB), U16(r7 + 0xFFFE), 1);
        hmark(h2 + H16(SYS + 4), y0 - 1, "2", U16(r7 - 0xB), U16(r7 + 0xFFFE), 2);
    }
    if (B8(SYS + 0x803)) {
        p = clamp_y((int)(v1 + y0 - 1));
        vmark(p);
        half_42e68be2_set(2);
        word_42e68c74_set(0x42E65418u);
        draw_colour_set(0);
        text_draw(0x3A6, U16(p + 0xFFFB), "V1");
        p = clamp_y((int)(v2 + y0 - 1));
        vmark(p);
        half_42e68be2_set(1);
        word_42e68c74_set(0x42E65418u);
        draw_colour_set(0);
        text_draw(0x3A5, U16(p + 0xFFFB), "V2");
    }
}
""", src="trace")
