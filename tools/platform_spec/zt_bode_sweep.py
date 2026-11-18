# Bode plot measurement: the frequency sweep, the measurement of one point (generator set, input
# ranges stepped until both amplitudes fit), the phase from the rising crossings of the two inputs,
# the calibration sweep, the vertical auto-scale and the drawing of the two curves with the legend
# and the "no signal" messages.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wt(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define BB 0x42E180C8u
#define GEN 0x42E3F984u
#define DREC 0x42E554B0u
#define TOUCH 0x42E68BB0u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W(a) (*(volatile u32 *)(a))
#define DD(a) (*(volatile double *)(a))
#define BH(o) H16(BB + (o))
#define BW(o) W(BB + (o))
#define BD(o) DD(BB + (o))
#define U16(v) ((unsigned short)(v))
"""

LIM = """
static const union { unsigned long long u; double d; } LIM = { 0x40B3880000000001ULL };   /* 5000.000000000001 */
"""

wt(0x40039254, 968, "bode_curves_draw",
   "Unless touch byte +0x17 is set: draws the gain curve (0xFF8800, pixel rows u16 0x42E180C8+0x8D20[i]) and the phase curve (0x0088FF, +0x9CC0[i]) as vertical segments between neighbouring points for x = 3 .. width - 3, "
   "then a legend box (0x55000000) at the top right with \"dB\" and \"deg\", and, unless both bytes +0xAD19 / +0xAD1A are set (input A / input B reached the level in the last sweep), a message box centred on the plot "
   "with UI text 129 (neither), 130 (only +0xAD1A) or 131 (only +0xAD19), in the colours SYS+0x3DD550 / +0x3DD554, the text moved by the entry's signed bytes +0x194 / +0x195.", COM + """
void draw_colour_set(u32 c);
void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void vline_draw(u32 x, u32 y0, u32 y1);
void func_4002ecc4_span(u32 y, u32 x0, u32 x1);
void text_draw(u32 x, u32 y, const char *s);
u32 text_width(const char *s);

static void curve(u32 tab, u32 colour)
{
    u32 x0 = BH(0), y0 = BH(2), a, b;
    int i;
    draw_colour_set(colour);
    for (i = 3; i < (int)BH(4) - 2; i++) {
        a = H16(BB + tab + 2 * i);
        b = H16(BB + tab + 2 * i + 2);
        if (a >= b)
            vline_draw(U16(x0 + i), U16(b + y0), U16(a + y0));
        else
            vline_draw(U16(x0 + i), U16(a + y0), U16(b + y0));
    }
}

static void message(u32 e)
{
    volatile unsigned char *t = (volatile unsigned char *)(0x42C2FE74u + e * 0x1A0u);
    u32 tw = text_width(*(const char *volatile *)t), x, y;
    x = BH(0) + ((u32)(BH(4) - tw - 0x28) >> 1);
    y = BH(2) + ((int)BH(6) - 0x28) / 2;
    draw_colour_set(W(SYS + 0x3DD550));
    rect_fill(U16(x), U16(y), U16(tw + 0x28), 0x28);
    draw_colour_set(W(SYS + 0x3DD554));
    text_draw(U16(x + (signed char)t[0x194] + 0x14), U16(y + (signed char)t[0x195] + 0xD),
              *(const char *volatile *)t);
}

void bode_curves_draw(void)
{
    u32 x, y;
    if (B8(TOUCH + 0x17))
        return;
    curve(0x8D20, 0xFF8800);
    curve(0x9CC0, 0x88FF);
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    x = BH(0) + BH(4) - 0x40;
    y = BH(2) + 2;
    draw_colour_set(0x55000000);
    rect_fill(U16(x), U16(y), 0x3E, 0x30);
    draw_colour_set(0xFF8800);
    func_4002ecc4_span(U16(y + 0xC), U16(x + 5), U16(x + 0x14));
    text_draw(U16(x + 0x1E), U16(y + 5), "dB");
    draw_colour_set(0x88FF);
    y += 0x17;
    func_4002ecc4_span(U16(y + 0xC), U16(x + 5), U16(x + 0x14));
    text_draw(U16(x + 0x1E), U16(y + 5), "deg");
    if (B8(BB + 0xAD19)) {
        if (B8(BB + 0xAD1A) == 0)
            message(131);
    } else
        message(B8(BB + 0xAD1A) ? 130 : 129);
}
""")

wt(0x4003963C, 672, "bode_autoscale",
   "Unless touch byte +0x17 is set: fits the n = word +0x5C points of the last sweep into 0.8 x the plot height H (u16 +6). Gain (doubles +0x3EE8[i], dB): scale = 0.8H x 10000 / (max - min of the values x 10000), at most 25 when it would be 26 or more; "
   "pixel row = (int)(v x scale) + H/2 - c with c = (min + max) x scale / 20000, kept in 3..H, into u16 +0x8D20[i]; the axis range goes to +0x10 (top) / +0x18 (bottom). Phase (s16 +0x7D68[i], degrees): the same with a spread of at least 50000 and a scale of at most 3 (min / max start at 0x3FFFFFFF / 0xC0000001), "
   "rows into +0x9CC0[i], range into +0x20 / +0x28. With n = 0 the sums wrap as in the stock code.", COM + """
static int clamp_row(int p)
{
    int h;
    if (p <= 2)
        return 3;
    h = BH(6);
    return h > p ? p : h;
}

void bode_autoscale(void)
{
    u32 H, half, n, k;
    int hh, mx, mn, v, c, lim;
    double base, sc, hd;
    if (B8(TOUCH + 0x17))
        return;
    H = BH(6);
    half = H >> 1;
    hh = (int)((double)H * 0.8);
    n = BW(0x5C);
    mx = (int)0xC0000001u;
    mn = 0x3FFFFFFF;
    for (k = 0; k < n; k++) {
        v = (int)(BD(0x3EE8 + 8 * k) * 10000.0);
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
        BH(0x8D20 + 2 * k) = (unsigned short)clamp_row((int)((u32)(int)(BD(0x3EE8 + 8 * k) * sc) + (half - (u32)c)));
    BD(0x18) = 0.0;
    hd = (double)(int)half;
    {
        double Hd = (double)BH(6), cd = (double)c, lo = (cd - hd) / sc, hi = (Hd - hd + cd) / sc;
        BD(0x18) = lo;
        BD(0x10) = hi;
    }
    mx = (int)0xC0000001u;
    mn = 0x3FFFFFFF;
    for (k = 0; k < n; k++) {
        v = (short)BH(0x7D68 + 2 * k) * 10000;
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
        BH(0x9CC0 + 2 * k) = (unsigned short)clamp_row((int)((u32)(int)((double)(short)BH(0x7D68 + 2 * k) * sc) + (half - (u32)c)));
    BD(0x28) = 0.0;
    {
        double Hd = (double)BH(6), cd = (double)c, lo = (cd - hd) / sc, hi = (Hd - hd + cd) / sc;
        BD(0x28) = lo;
        BD(0x20) = hi;
    }
}
""")

wt(0x40039C08, 676, "bode_phase_measure",
   "Phase of sweep point idx: walks the two capture buffers (u16 samples at 0x42AC7CF4 and +0x62250, shifted right by SYS+0x1BC) from 50 x step to SYS+0x2B0 - 50 x step in steps of SYS+0x558, finds the rising crossings of the level u16 SYS+0xE on each input "
   "(at least 8 steps apart), pairs a crossing of A with the following one of B and averages their distance d; period p = mean of (last - first + 1) / (crossings - 1) of both inputs; phase = 360 d / p minus the calibration offset double 0x42E180C8+0xAD10, "
   "wrapped into -180..180, stored as s16 at +0x7D68[idx]. Divisions by 0 give 0 (ARM sdiv). The shift is assumed below 32.", COM + """
static int sdiv0(int a, int b)
{
    return b ? a / b : 0;
}

void bode_phase_measure(u32 idx)
{
    u32 step = W(SYS + 0x558), start = 50 * step, end, pos, sh, a, b, prev_a = 0, prev_b = 0;
    int level = H16(SYS + 0xE);
    volatile unsigned short *p = (volatile unsigned short *)(0x42AC7CF4u + 2 * start);
    int na = 0, nb = 0, count = 0, sum = 0, first_a = 0, last_a = 0, first_b = 0, last_b = 0;
    int got_a = 0, got_b = 0, fl_a = 0, fl_b = 0, cnt_a = 8, cnt_b = 8, k = 0, avg, pa, pb, ph, v;
    end = W(SYS + 0x2B0) - start;
    sh = W(SYS + 0x1BC);
    if (start >= end) {
        na = 2;
        nb = 2;
    } else {
        pos = start;
        do {
            a = (unsigned short)(p[0] >> sh);
            b = (u32)H16((u32)p + 0x62250u) >> sh;
            if (k > 0) {
                if (cnt_a >= 8) {
                    if ((int)prev_a < level && (int)a >= level) {
                        if (!got_a) {
                            first_a = (int)pos;
                            got_a = 1;
                        }
                        fl_a = 1;
                        na++;
                        last_a = (int)pos;
                        cnt_a = 0;
                    }
                } else
                    cnt_a++;
                if (cnt_b >= 8) {
                    if (level > (int)(prev_b & 0xFFFF) && level <= (int)(b & 0xFFFF)) {
                        if (!got_b) {
                            first_b = (int)pos;
                            got_b = 1;
                        }
                        fl_b = 1;
                        nb++;
                        last_b = (int)pos;
                        cnt_b = 0;
                    }
                } else
                    cnt_b++;
                if (fl_a && fl_b) {
                    fl_a = 0;
                    fl_b = 0;
                    count++;
                    sum = (int)((u32)last_b + (u32)sum - (u32)last_a);
                }
            }
            prev_b = b;
            prev_a = a;
            step = W(SYS + 0x558);
            k++;
            pos += step;
            p += step;
        } while (pos < end);
        if (na <= 1)
            na = 2;
        if (nb <= 1)
            nb = 2;
    }
    avg = sdiv0(sum, count);
    pa = sdiv0(last_a - first_a + 1, na - 1);
    pb = sdiv0(last_b - first_b + 1, nb - 1);
    ph = sdiv0((int)(360u * (u32)avg), (pa + pb) / 2);
    v = (int)((double)ph - BD(0xAD10));
    if (v >= 181)
        v -= 360;
    else if (v < -180)
        v += 360;
    BH(0x7D68 + 2 * (idx & 0xFFFF)) = (unsigned short)v;
}
""")

wt(0x4003A044, 652, "bode_point_measure",
   "One sweep point at f Hz: generator frequency word 0x42E3F984+0x2EFC = f, gen_update, 1 ms; input coupling bytes SYS+0x46 / +0x10A = 2 below 5 MHz, else 1; time-base SYS+0x588 = the first index 14..33 whose table entry (u64 SYS+0xC40 + 8 x (i - 14)) x u16 SYS+0x1E reaches 8 x 10^12 / f (else 0x21). "
   "Calibration: when the first word of record D (0x42E554B0) is below f, the entry whose frequency (words +0x10[i]) is the first >= f gives the gain ratio (+0x4C0[i]) and the phase offset (+0xE20[i]) in 0x42E180C8+0xAD08 / +0xAD10, else 1.0 / 0. "
   "Then, until both inputs are done: runtime_event_service unless byte +0xAD18 is set (touch byte +0x15 aborts: +0x47 = 0), outputs_apply, acquisition_dispatcher; for each input the range byte (SYS+0x41 for A, +0x105 for B) goes down while the amplitude word (SYS+0xB30 / +0xB4C) is above 0.8 x u16 SYS+0xA "
   "and up (to at most 11) while it is below 0.3 x; otherwise the 64-bit amplitude (SYS+0x910 / +0x970) is taken as a double into +0xACF8 / +0xAD00. At the end +0xAD00 is divided by the calibration ratio.", COM + """
void gen_update(void);
void delay_ms(u32 ms);
void runtime_event_service(void);
void outputs_apply(void);
void acquisition_dispatcher(void);
unsigned long long __aeabi_uldivmod(unsigned long long a, unsigned long long b);

void bode_point_measure(u32 f)
{
    u32 hmax, hmin, f2, n, e, a, bv, g;
    int i, done_a = 0, done_b = 0;
    unsigned long long q8;
    double h = (double)H16(SYS + 0xA);
    hmax = (u32)(h * 0.8);
    hmin = (u32)(h * 0.3);
    W(GEN + 0x2EFC) = f;
    gen_update();
    delay_ms(1);
    f2 = W(GEN + 0x2EFC);
    if ((int)f2 < 5000000) {
        B8(SYS + 0x46) = 2;
        B8(SYS + 0x10A) = 2;
    } else {
        B8(SYS + 0x46) = 1;
        B8(SYS + 0x10A) = 1;
    }
    q8 = __aeabi_uldivmod(1000000000000ULL, (unsigned long long)(long long)(int)f2) << 3;
    n = H16(SYS + 0x1E);
    B8(SYS + 0x588) = 0x21;
    for (i = 14; i < 0x22; i++)
        if (*(volatile unsigned long long *)(SYS + 0xC40 + 8 * (i - 14)) * n >= q8) {
            B8(SYS + 0x588) = (unsigned char)i;
            break;
        }
    if (W(DREC) >= f) {
        BD(0xAD08) = 1.0;
        BD(0xAD10) = 0.0;
    } else {
        n = W(DREC + 0xC);
        for (i = 0;;) {
            if (W(DREC + 0x10 + 4 * i) >= f)
                break;
            i++;
            if ((u32)i > n)
                goto measure;
        }
        e = W(DREC + 0x10 + 4 * i);
        if (f - e < e - f)
            i--;
        BD(0xAD08) = DD(DREC + 0x4C0 + 8 * i);
        BD(0xAD10) = DD(DREC + 0xE20 + 8 * i);
    }
measure:
    for (;;) {
        if (B8(BB + 0xAD18) == 0) {
            runtime_event_service();
            if (B8(TOUCH + 0x15)) {
                B8(BB + 0x47) = 0;
                break;
            }
        }
        outputs_apply();
        acquisition_dispatcher();
        a = W(SYS + 0xB30);
        bv = W(SYS + 0xB4C);
        if (!done_a) {
            g = B8(SYS + 0x41);
            if (a > hmax && g != 0)
                B8(SYS + 0x41) = (unsigned char)(g - 1);
            else if (a <= hmax && a < hmin && g <= 10)
                B8(SYS + 0x41) = (unsigned char)(g + 1);
            else {
                done_a = 1;
                BD(0xACF8) = (double)*(volatile long long *)(SYS + 0x910);
            }
        }
        if (!done_b) {
            g = B8(SYS + 0x105);
            if (bv > hmax && g != 0)
                B8(SYS + 0x105) = (unsigned char)(g - 1);
            else if (bv <= hmax && bv < hmin && g <= 10)
                B8(SYS + 0x105) = (unsigned char)(g + 1);
            else {
                done_b = 1;
                BD(0xAD00) = (double)*(volatile long long *)(SYS + 0x970);
            }
        }
        if (done_a && done_b)
            break;
    }
    BD(0xAD00) = BD(0xAD00) / BD(0xAD08);
}
""")

SWEEPDECL = """
void bode_point_measure(u32 f);
void bode_phase_measure(u32 idx);
void runtime_event_service(void);
double log10(double x);
""" + LIM + """
static double point_ratio(void)
{
    double a = BD(0xACF8), b = BD(0xAD00), r;
    r = b / a;
    if (a >= LIM.d)
        B8(BB + 0xAD19) = 1;
    if (b >= LIM.d)
        B8(BB + 0xAD1A) = 1;
    return r;
}
"""

wt(0x4003A300, 676, "bode_sweep_run",
   "Unless touch byte +0x17 is set: runs a Bode sweep from word 0x42E180C8+0x54 to +0x58 Hz in n = +0x5C log steps (+0x60 = linear step, +0x8D08 / +0x8D10 / +0x8D18 = log10 of start, end and span; the generator words 0x42E3F984+0x2F00/+0x2F04 get +0x4C/+0x50). "
   "For i = 0..n: shows the progress \"NN%\" at (903, 510) (0..99; at 100 the previous text stays), bode_point_measure(10^(log start + i x step)), ratio B / A into +0x68[i] and 20 log10 into +0x3EE8[i], bode_phase_measure(i); touch byte +0x15 stops the sweep. "
   "Byte +0xAD19 / +0xAD1A is set when the A / B amplitude reaches 5000; unless both are set the results are reset to ratio 1, 0 dB, phase 0.", COM + SWEEPDECL + """
double pow(double x, double y);
void word_42e68c74_set(u32 a);
void half_42e68be2_set(u32 a);
void draw_target_default(void);
void fb_target_back(void);
void dcache_clean_inv_all_t(void);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void text_draw(u32 x, u32 y, const char *s);

void bode_sweep_run(void)
{
    char buf[4];
    u32 i, n, pct;
    double l0, l1, span, step, r;
    if (B8(TOUCH + 0x17))
        return;
    B8(BB + 0x47) = 1;
    B8(BB + 0xAD18) = 0;
    BD(0x60) = (double)(int)(BW(0x58) - BW(0x54)) / (double)(int)BW(0x5C);
    W(GEN + 0x2F00) = BW(0x4C);
    W(GEN + 0x2F04) = BW(0x50);
    B8(BB + 0xAD19) = 0;
    B8(BB + 0xAD1A) = 0;
    l0 = log10((double)(int)BW(0x54));
    l1 = log10((double)(int)BW(0x58));
    span = l1 - l0;
    BD(0x8D10) = l1;
    BD(0x8D08) = l0;
    BD(0x8D18) = span;
    step = span / (double)(int)BW(0x5C);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    for (i = 0; (n = BW(0x5C)) >= i; i++) {
        pct = n ? 100 * i / n : 0;
        if ((int)pct < 10) {
            buf[0] = (char)('0' + pct);
            buf[1] = '%';
            buf[2] = 0;
        } else if ((int)pct < 100) {
            buf[0] = (char)('0' + (int)pct / 10);
            buf[1] = (char)('0' + (int)pct % 10);
            buf[2] = '%';
            buf[3] = 0;
        }
        draw_target_default();
        draw_colour_set(0);
        rect_fill(0x384, 0x1FC, 0x23, 0x12);
        draw_colour_set(0xAAAAAA);
        text_draw(0x387, 0x1FE, buf);
        fb_target_back();
        dcache_clean_inv_all_t();
        bode_point_measure((u32)pow(10.0, BD(0x8D08) + (double)i * step));
        r = point_ratio();
        BD(0x68 + 8 * i) = r;
        BD(0x3EE8 + 8 * i) = log10(r) * 20.0;
        bode_phase_measure(i & 0xFFFF);
        runtime_event_service();
        if (B8(TOUCH + 0x15)) {
            B8(BB + 0x47) = 0;
            break;
        }
    }
    if (B8(BB + 0xAD19) == 0 || B8(BB + 0xAD1A) == 0)
        for (i = 0;; ) {
            BD(0x68 + 8 * i) = 1.0;
            BD(0x3EE8 + 8 * i) = log10(1.0) * 20.0;
            BH(0x7D68 + 2 * i) = 0;
            i++;
            if (BW(0x5C) < i)
                break;
        }
}
""")

wt(0x40039EB8, 380, "bode_calibrate",
   "Calibration sweep: trace_defaults_37b48 (which fills the frequencies of record D 0x42E554B0), byte 0x42E180C8+0xAD18 = 1 (no event service during the points), +0x47 = 1, +0x4C = 1000, +0x50 = 0 (also into the generator words 0x42E3F984+0x2F00/04), touch bytes +0x16 / +0x15 = 0; "
   "resets the calibration (ratio +0x4C0[i] = 1.0, phase +0xE20[i] = 0 for i = 0 .. word D+0xC), input ranges SYS+0x41 / +0x105 = 11, then measures every frequency D+0x10[i] (ratio into 0x42E180C8+0x68[i], phase into +0x7D68[i]) and copies them into the calibration "
   "(ratio, phase as a double). The record is saved (settings_d_save) only when both inputs reached 5000. Ends with touch byte +0x16 = 1.", COM + SWEEPDECL + """
void trace_defaults_37b48(void);
void settings_d_save(void);

void bode_calibrate(void)
{
    u32 i;
    trace_defaults_37b48();
    B8(BB + 0xAD18) = 1;
    B8(BB + 0x47) = 1;
    BW(0x4C) = 1000;
    BW(0x50) = 0;
    W(GEN + 0x2F00) = 1000;
    W(GEN + 0x2F04) = 0;
    B8(TOUCH + 0x16) = 0;
    B8(TOUCH + 0x15) = 0;
    for (i = 0;; ) {
        DD(DREC + 0x4C0 + 8 * i) = 1.0;
        W(DREC + 0xE20 + 8 * i) = 0;
        W(DREC + 0xE24 + 8 * i) = 0;
        i++;
        if (W(DREC + 0xC) < i)
            break;
    }
    B8(BB + 0xAD19) = 0;
    B8(BB + 0xAD1A) = 0;
    B8(SYS + 0x41) = 11;
    B8(SYS + 0x105) = 11;
    for (i = 0;; ) {
        bode_point_measure(W(DREC + 0x10 + 4 * i));
        BD(0x68 + 8 * i) = point_ratio();
        bode_phase_measure(i & 0xFFFF);
        i++;
        if (W(DREC + 0xC) < i)
            break;
    }
    for (i = 0;; ) {
        DD(DREC + 0x4C0 + 8 * i) = BD(0x68 + 8 * i);
        DD(DREC + 0xE20 + 8 * i) = (double)(short)BH(0x7D68 + 2 * i);
        i++;
        if (W(DREC + 0xC) < i)
            break;
    }
    if (B8(BB + 0xAD19) && B8(BB + 0xAD1A))
        settings_d_save();
    B8(TOUCH + 0x16) = 1;
}
""")
