# Signal generator waveform builder: one period of the selected waveform into the output table
# (G+0xFB6, length G+0x2EF6, amplitude/offset from the settings block) and the preview table
# (GEN_BUF, one period of length / count samples, centred on half the top value).
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wg(addr, size, name, desc, body, src="generator"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


wg(0x4004D880, 5096, "gen_wave_build",
   "Builds the generator tables for the waveform GEN_WAVE (G+0x2EFA). Output table G+0xFB6 (n = u16 G+0x2EF6 samples, amplitude A = "
   "settings +0x34, offset O = +0x4C, divisor D = +0x5C, duty +0xC per mille) and preview table GEN_BUF (per = length / count of the "
   "descriptor G+0xFAE, half = top / 2). Frequency 0: the output table is filled with O. 0 sine (above 17/20/22/25 MHz n becomes "
   "6/5/4/2 with phase 0.4/0.2/0.5/pi/2), 1 pulse (duty limited so that each level has at least 3 samples), 2 triangle, 3 ramp up, "
   "4 ramp down (writes one sample past the end, index n .. 1), 5 staircase (5..10 steps), 6 half-wave sine, 7 sine hump, "
   "8 exponential (exp(5.995 x) x 40), 9 sinc-like x^2 sin(5x), 10 square-root peak, 11 sum of 5 harmonics (0.79 offsets), "
   "12 sum of 4 phase-shifted harmonics (0.365 offsets), 13 DC, 14 arbitrary (normalize + resample both tables). "
   "Skipped while the touch service is busy.", r"""
double sin(double x);
double exp(double x);
double sqrt(double x);
void fill_u16(unsigned short *d, unsigned short v, u32 n);
void gen_normalize(void);
void gen_resample(void);
void gen_normalize2(void);
void gen_resample2(void);

#define GW    ((volatile int *)(GEN_G + 0x2EFC))
#define GU(i) ((u32)GW[i])
#define NOUT  G16(0x2EF6)
#define TWO_PI 6.283185307179586

void gen_wave_build(void)
{
    volatile int *g = GW;
    unsigned short *o = GEN_BUF2, *b = GEN_BUF;
    int per, half, i, j, n, h;
    double st, ph, d, k, s;
    if (g_touch->serviced != 0)
        return;
    per = (int)G16(0xFAE) / (int)G16(0xFAE + 6);
    half = G16(0xFAE + 2) >> 1;
    if (g[0] == 0) {
        fill_u16(o, (unsigned short)g[19], NOUT);
        return;
    }
    switch (GEN_WAVE) {
    case 0:
        if (g[0] >= 25000000) { NOUT = 2; ph = 1.5707963267948966; }
        else if (g[0] >= 22000000) { NOUT = 4; ph = 0.5; }
        else if (g[0] >= 20000000) { NOUT = 5; ph = 0.2; }
        else if (g[0] >= 17000000) { NOUT = 6; ph = 0.4; }
        else { NOUT = NOUT; ph = 0.0; }
        st = TWO_PI / (double)(u32)NOUT;
        for (i = 0; i < (int)NOUT; i++)
            o[i] = (int)((double)GU(13) * sin(ph + (double)i * st));
        for (i = 0; i < (int)NOUT; i++)
            o[i] = o[i] + g[19];
        st = TWO_PI / (double)per;
        for (i = 0; i < per; i++)
            b[i] = (int)((double)half + (double)half * sin((double)i * st));
        return;
    case 1: {
        int dt;
        n = NOUT;
        dt = g[3];
        if (n * dt / 1000 <= 2)
            dt = 3000 / n;
        if ((1000 - dt) * n / 1000 <= 2)
            dt = 1000 - 3000 / n;
        g[3] = dt;
        for (i = 0; i < (int)NOUT; i++) {
            int t = (int)NOUT * g[3], q = t / 1000;
            if (t - q * 1000 >= 500)
                q++;
            o[i] = i < q ? g[13] + g[19] : g[19] - g[13];
        }
        for (i = 0; i < per; i++) {
            int t = g[3] * per, q = t / 1000;
            if (t - q * 1000 >= 500)
                q++;
            if (q <= 0)
                q = 1;
            else if (per - 2 < q)
                q = per - 2;
            b[i] = i < q ? half << 1 : 0;
        }
        return;
    }
    case 2:
        h = NOUT >> 1;
        d = (double)GU(13) * 2.0 / (double)h;
        for (i = 0; i < (int)NOUT; i++) {
            if (i < h)
                o[i] = (int)((double)i * d) + g[19] - g[13];
            else
                o[i] = g[13] + g[19] - (int)((double)(i - h) * d);
        }
        h = per / 2;
        d = (double)half * 2.0 / (double)h;
        for (i = 0; i < per; i++)
            b[i] = i < h ? (int)((double)i * d) : (half << 1) - (int)((double)(i - h) * d);
        return;
    case 3:
        d = (double)GU(13) * 2.0 / (double)(u32)NOUT;
        for (i = 0; i < (int)NOUT; i++)
            o[i] = (int)((double)i * d) + g[19] - g[13];
        d = (double)half * 2.0 / (double)per;
        for (i = 0; i < per; i++)
            b[i] = (int)((double)i * d);
        return;
    case 4:
        d = (double)GU(13) * 2.0 / (double)(u32)NOUT;
        for (i = 0; i < (int)NOUT; i++)
            o[NOUT - i] = (int)((double)i * d) + g[19] - g[13];
        d = (double)half * 2.0 / (double)per;
        for (i = 0; i < per; i++)
            b[per - i] = (int)((double)i * d);
        return;
    case 5: {
        int ns, q;
        n = NOUT;
        if ((u32)(n - 25) > 24) ns = 10;
        else ns = 5 + (n - 25) / 5;
        if (n < 200 && n - n / ns * ns > 5)
            ns++;
        q = n / ns;
        d = (double)((GU(13) << 1) / (u32)(ns - 1));
        for (i = 0; i < (int)NOUT; i++)
            o[i] = (int)((double)GU(19) + (double)(i / q) * d - (double)GU(13));
        d = (double)((half << 1) / (ns - 1));
        q = per / ns + 1;
        k = (double)half;
        for (i = 0; i < per; i++)
            b[i] = (int)(k + (double)(i / q) * d - k);
        return;
    }
    case 6:
        h = NOUT >> 1;
        st = TWO_PI / (double)(u32)NOUT;
        for (i = 0; i < (int)NOUT; i++) {
            int v = g[19];
            s = sin((double)i * st);
            if (i < h)
                v += (int)((double)GU(13) * s * 2.0);
            o[i] = v - g[13];
        }
        st = TWO_PI / (double)per;
        h = per / 2;
        for (i = 0; i < per; i++) {
            s = sin((double)i * st);
            b[i] = i < h ? (int)((double)half * s * 2.0) : 0;
        }
        return;
    case 7:
        st = TWO_PI / (double)(u32)NOUT * 0.5;
        for (i = 0; i < (int)NOUT; i++) {
            d = (double)GU(19);
            s = sin((double)i * st);
            o[i] = (int)(d + (double)GU(13) * s * 2.0 - (double)GU(13));
        }
        st = TWO_PI / (double)per * 0.5;
        k = (double)half;
        for (i = 0; i < per; i++) {
            s = sin((double)i * st);
            b[i] = (int)(k + (double)half * s * 2.0 - k);
        }
        return;
    case 8:
        st = 5.995 / (double)(u32)NOUT;
        d = (double)GU(13) / (double)GU(23);
        for (i = 0; i < (int)NOUT; i++) {
            k = (double)GU(19);
            s = exp((double)i * st) * 40.0;
            o[i] = (int)(k + s * d - (double)GU(13));
        }
        st = 5.995 / (double)per;
        k = (double)half;
        d = k / (double)GU(23);
        for (i = 0; i < per; i++) {
            s = exp((double)i * st) * 40.0;
            b[i] = (int)(k + s * d - k);
        }
        return;
    case 9:
        d = (double)(u32)NOUT;
        st = TWO_PI / d * 5.0;
        k = 1.224 / (d * d / 10.0);
        for (i = 0; i < (int)NOUT; i++) {
            double w = (double)i * (double)i / 10.0 * k;
            s = sin((double)i * st);
            o[i] = (int)(w * s * (double)GU(13));
        }
        for (i = 0; i < (int)NOUT; i++)
            o[i] = (int)((double)o[i] + (double)GU(19) * 1.102);
        d = (double)per;
        st = TWO_PI / d * 5.0;
        k = 1.224 / (d * d / 10.0);
        for (i = 0; i < per; i++) {
            double w = (double)i * (double)i / 10.0 * k;
            s = sin((double)i * st);
            b[i] = (int)(w * s * (double)half);
        }
        if (per > 0) {
            d = (double)half * 1.102;
            j = per;
            i = 0;
            do {
                b[i] = (int)((double)b[i] + d);
                i++;
            } while (--j);
        }
        return;
    case 10: {
        u32 nn = NOUT;
        h = nn >> 1;
        st = (double)((nn & 1) ? 500000000 / (int)nn : 520000000 / (int)nn);
        d = (double)GU(13) / (double)GU(23);
        for (i = 0; i < (int)NOUT; i++) {
            int x = i < h ? i : (int)NOUT - i;
            o[i] = (int)(sqrt((double)x * st) * d) + g[19] - g[13];
        }
        st = 500000000.0 / (double)per;
        d = (double)half / (double)GU(23);
        h = per / 2;
        for (i = 0; i < per; i++) {
            int x = i < h ? i : per - i;
            b[i] = (int)(sqrt((double)x * st) * d);
        }
        return;
    }
    case 11:
        st = TWO_PI / (double)(u32)NOUT;
        d = (double)(GU(13) * 2015 / GU(23));
        for (i = 0; i < (int)NOUT; i++) {
            double x = (double)i * st, s1, s2, s3, s4, a;
            s1 = sin(x); s2 = sin(x * 2.0); s3 = sin(x * 3.0); s4 = sin(x * 4.0);
            a = (double)GU(19);
            a = a + (s1 + 0.79) * d;
            a = a + (s2 + 0.79) * d;
            a = a + (s3 + 0.79) * d;
            a = a + (s4 + 0.79) * d;
            a = a + (sin(x * 5.0) + 0.79) * d;
            o[i] = (int)(a - (double)GU(13));
        }
        d = (double)((u32)half * 2015 / GU(23));
        st = TWO_PI / (double)per;
        k = (double)half;
        for (i = 0; i < per; i++) {
            double x = (double)i * st, s1, s2, s3, s4, a;
            s1 = sin(x); s2 = sin(x * 2.0); s3 = sin(x * 3.0); s4 = sin(x * 4.0);
            a = k;
            a = a + (s1 + 0.79) * d;
            a = a + (s2 + 0.79) * d;
            a = a + (s3 + 0.79) * d;
            a = a + (s4 + 0.79) * d;
            a = a + (sin(x * 5.0) + 0.79) * d;
            b[i] = (int)(a - k);
        }
        return;
    case 12:
        st = TWO_PI / (double)(u32)NOUT;
        d = (double)(GU(13) * 3150 / GU(23));
        for (i = 0; i < (int)NOUT; i++) {
            double x = (double)i * st, s1, s2, s3, a;
            s1 = sin(x); s2 = sin(4.72 + x * 2.0); s3 = sin(9.44 + x * 3.0);
            a = (double)GU(19);
            a = a + (s1 + 0.365) * d;
            a = a + (s2 + 0.365) * d;
            a = a + (s3 + 0.365) * d;
            a = a + (sin(14.16 + x * 4.0) + 0.365) * d * 0.7;
            o[i] = (int)(a - (double)GU(13));
        }
        d = (double)((u32)half * 3150 / GU(23));
        st = TWO_PI / (double)per;
        k = (double)half;
        for (i = 0; i < per; i++) {
            double x = (double)i * st, s1, s2, s3, a;
            s1 = sin(x); s2 = sin(4.72 + x * 2.0); s3 = sin(9.44 + x * 3.0);
            a = k;
            a = a + (s1 + 0.365) * d;
            a = a + (s2 + 0.365) * d;
            a = a + (s3 + 0.365) * d;
            a = a + (sin(14.16 + x * 4.0) + 0.365) * d * 0.7;
            b[i] = (int)(a - k);
        }
        return;
    case 13:
        for (i = 0; i < (int)NOUT; i++)
            o[i] = g[19];
        for (i = 0; i < per; i++)
            b[i] = half;
        return;
    case 14:
        gen_normalize2();
        gen_resample2();
        gen_normalize();
        gen_resample();
        return;
    default:
        return;
    }
}
""")
