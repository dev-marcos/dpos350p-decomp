/*
 * acquisition_timing_setup -- stock 0x4003A984 (3636 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Third step of acquisition_apply. With SYS+0x652 set the time base SYS+0x588 is limited to 0x20; timebase_period_load, zoom_entry_select, SYS+0x101 = 1, and nothing more while SYS+0x3C is set. Time base index t = SYS+0x588 - 11 (0 .. 31; other values keep the old fields) picks from one of two tables (divisor SYS+0x1B0 = 1 or not) the s64 sample period SYS+0x538, the block count half SYS+0x550, the step word SYS+0x558 and the decimation word SYS+0x55C (= SYS+0x554 x k). The clock generator (ms5351m_b, then 5 ms) is reprogrammed when word SYS+0x5F0 differs from 100 MHz (t = 0) or 0x05A92F37. Then: SYS+0x530 = period x decimation / SYS+0x554, SYS+0x528 = period / SYS+0x1B1; block count / 4 in roll mode (SYS+0x27A = 2, also into SYS+0x582) and 1 in single / mode-1 / SYS+0x54F; the smallest k with decimation x k divisible by SYS+0x554 (SYS+0x560 = k, SYS+0x564 = quotient); block sizes SYS+0x2B0 .. +0x2C0 from the plot width SYS+8; the trigger hold-off s64 SYS+0x30 (1e10 / SYS+0x590 x SYS+0x1C x step / blocks x SYS+0x554 / decimation / SYS+0x1B1, at least 2 x block when SYS+0x275, else 1.2 x block; single mode and SYS+0x652 have their own values); chunk count SYS+0x644 (smallest k with block / k <= SYS+0x642) and chunk size SYS+0x640; the view position s64 SYS+0x570 clamped to +-SYS+0xC steps (SYS+0x590); the trigger multiplier SYS+0x282 from SYS+0x284 (1 / 2 / 3 / 6) and the pre-trigger words SYS+0x294 / +0x298 / half +0x276 (0.95 x half the record); outside single mode the window SYS+0x568 / +0x56C (shifted by 2^(SYS+0x660 - 6) x 3 or 1.5) clamps the position again; retained_inside_current_sc1_3_t6; ratios SYS+0xE0 / +0x1A0 (32 x s64 SYS+0xD8 / SYS+0x50, + 0x198 / + 0x110, 32 when not above); bandwidth limits s64 SYS+0x68 / +0x128 from the coupling bytes (0 / 1 / 2: 350 / 150 / 20 MHz) and, with SYS+0x660 set, at most SYS+0x530 / n / 1000 / 2.24 (also into SYS+0xF8, n into SYS+0x662); the trigger source SYS+0x27C follows the only channel on.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define Q64(a) (*(volatile unsigned long long *)(a))
#define S64(a) (*(volatile long long *)(a))

/* Thumb runtime helpers, called through their Thumb addresses like the stock BLX */
typedef double (*ul2d_fn)(unsigned long long) __attribute__((pcs("aapcs")));
#define UL2D(v) (((ul2d_fn)0x4005F731u)(v))
typedef double (*pow_fn)(double, double);
#define POW(a, b) (((pow_fn)0x4005D4A5u)((a), (b)))

void timebase_period_load(void);
void zoom_entry_select(void);
void ms5351m_b(u32 hz, u32 old);
void retained_inside_current_sc1_3_t6(void);

struct tb_row {
    unsigned long long period;
    unsigned char n, w, k;
};

static const struct tb_row TB1[32] = {
    {0xE8D4A51000ULL, 1, 50, 1},
    {0xDD30699C00ULL, 24, 1, 20},
    {0xDD30699C00ULL, 24, 1, 10},
    {0xDD30699C00ULL, 24, 1, 5},
    {0xDD30699C00ULL, 24, 1, 2},
    {0xDD30699C00ULL, 24, 1, 1},
    {0xDD30699C00ULL, 16, 2, 1},
    {0xDD30699C00ULL, 8, 5, 1},
    {0xDD30699C00ULL, 8, 10, 1},
    {0xDD30699C00ULL, 8, 20, 1},
    {0xDD30699C00ULL, 4, 50, 1},
    {0x6E9834CE00ULL, 4, 50, 1},
    {0x374C1A6700ULL, 4, 50, 1},
    {0x161E70F600ULL, 4, 50, 1},
    {0xB0F387B00ULL, 4, 50, 1},
    {0x5879C3D80ULL, 4, 50, 1},
    {0x2363E7F00ULL, 3, 50, 1},
    {0x11B1F3F80ULL, 2, 50, 1},
    {0x8D8F9FC0ULL, 1, 50, 1},
    {0x389FD980ULL, 1, 50, 1},
    {0x1C4FECC0ULL, 1, 50, 1},
    {0xE27F660ULL, 1, 50, 1},
    {0x5A995C0ULL, 1, 50, 1},
    {0x2D4CAE0ULL, 1, 50, 1},
    {0x16A6570ULL, 1, 50, 1},
    {0x90F560ULL, 1, 50, 1},
    {0x487AB0ULL, 1, 50, 1},
    {0x243D58ULL, 1, 50, 1},
    {0xE7EF0ULL, 1, 50, 1},
    {0x73F78ULL, 1, 50, 1},
    {0x39FBCULL, 1, 50, 1},
    {0x17318ULL, 1, 50, 1},
};

static const struct tb_row TB2[32] = {
    {0x746A528800ULL, 1, 25, 1},
    {0x6E9834CE00ULL, 24, 1, 40},
    {0x6E9834CE00ULL, 24, 1, 20},
    {0x6E9834CE00ULL, 24, 1, 10},
    {0x6E9834CE00ULL, 24, 1, 4},
    {0x6E9834CE00ULL, 24, 1, 2},
    {0x6E9834CE00ULL, 24, 1, 1},
    {0x5879C3D800ULL, 16, 2, 1},
    {0x6E9834CE00ULL, 8, 5, 1},
    {0x6E9834CE00ULL, 6, 10, 1},
    {0x6E9834CE00ULL, 4, 25, 1},
    {0x374C1A6700ULL, 4, 25, 1},
    {0x1BA60D3380ULL, 4, 25, 1},
    {0xB0F387B00ULL, 4, 25, 1},
    {0x5879C3D80ULL, 4, 25, 1},
    {0x2C3CE1EC0ULL, 4, 25, 1},
    {0x11B1F3F80ULL, 3, 25, 1},
    {0x8D8F9FC0ULL, 2, 25, 1},
    {0x46C7CFE0ULL, 1, 25, 1},
    {0x1C4FECC0ULL, 1, 25, 1},
    {0xE27F660ULL, 1, 25, 1},
    {0x713FB30ULL, 1, 25, 1},
    {0x2D4CAE0ULL, 1, 25, 1},
    {0x16A6570ULL, 1, 25, 1},
    {0xB532B8ULL, 1, 25, 1},
    {0x487AB0ULL, 1, 25, 1},
    {0x243D58ULL, 1, 25, 1},
    {0x121EACULL, 1, 25, 1},
    {0x73F78ULL, 1, 25, 1},
    {0x39FBCULL, 1, 25, 1},
    {0x1CFDEULL, 1, 25, 1},
    {0xB98CULL, 1, 25, 1},
};

static const u32 BW[3] = {350000000u, 150000000u, 20000000u};

static u32 ratio32(u32 a_off, u32 b_off)
{
    unsigned long long a = Q64(SYS + a_off), b = Q64(SYS + b_off);

    if (a <= b)
        return 0x20;
    return (u32)((long long)(a << 5) / (long long)b);
}

void acquisition_timing_setup(void)
{
    u32 tb, idx, d, n, fpv, m, h, p, b, lim, c1, c2, clk;
    int iv, lo, s;
    unsigned long long x, t, k, lim1, lim2, u;
    long long q, pos;
    const struct tb_row *e;

    if (B8(SYS + 0x652) && B8(SYS + 0x588) > 0x20)
        B8(SYS + 0x588) = 0x20;
    timebase_period_load();
    zoom_entry_select();
    B8(SYS + 0x101) = 1;
    if (B8(SYS + 0x3C))
        return;

    tb = B8(SYS + 0x588);
    idx = tb - 11;
    if (idx < 32) {
        e = (B8(SYS + 0x1B0) == 1 ? TB1 : TB2) + idx;
        Q64(SYS + 0x538) = e->period;
        H16(SYS + 0x550) = e->n;
        W32(SYS + 0x558) = e->w;
        W32(SYS + 0x55C) = W32(SYS + 0x554) * e->k;
    }
    clk = tb == 11 ? 100000000u : 0x05A92F37u;
    if (W32(SYS + 0x5F0) != clk) {
        ms5351m_b(clk, W32(SYS + 0x5F0));
        delay_ms(5);
    }

    x = Q64(SYS + 0x538);
    Q64(SYS + 0x530) = __aeabi_uldivmod(x * W32(SYS + 0x55C), W32(SYS + 0x554));
    Q64(SYS + 0x528) = __aeabi_uldivmod(x, B8(SYS + 0x1B1));
    if (B8(SYS + 0x27A) == 2) {
        n = H16(SYS + 0x550) >> 2;
        if (n == 0)
            n = 1;
        H16(SYS + 0x550) = (unsigned short)n;
        H16(SYS + 0x582) = (unsigned short)n;
    }
    if (B8(SYS + 0x28) || B8(SYS + 0x27A) == 1 || B8(SYS + 0x54F))
        H16(SYS + 0x550) = 1;

    fpv = W32(SYS + 0x55C);
    k = 1;
    if (fpv == W32(SYS + 0x554))
        q = 1;
    else {
        while ((long long)(fpv * k) % (long long)W32(SYS + 0x554) != 0)
            k++;
        q = (long long)(fpv * k) / (long long)W32(SYS + 0x554);
    }
    W32(SYS + 0x564) = (u32)q;
    W32(SYS + 0x560) = (u32)k;

    d = B8(SYS + 0x1B1);
    n = H16(SYS + 8) * W32(SYS + 0x558) / d + 1;
    m = W32(SYS + 0x62C) + n;
    W32(SYS + 0x2B0) = d * n;
    W32(SYS + 0x2BC) = m;
    W32(SYS + 0x2B8) = n;
    W32(SYS + 0x2B4) = d * m;
    W32(SYS + 0x2C0) = H16(SYS + 0x550) * W32(SYS + 0x2B0);

    x = __aeabi_uldivmod(10000000000ULL, Q64(SYS + 0x590));
    x = x * H16(SYS + 0x1C) * W32(SYS + 0x558);
    x = __aeabi_uldivmod(x, H16(SYS + 0x550));
    x = __aeabi_uldivmod(x * W32(SYS + 0x554), W32(SYS + 0x55C));
    x = __aeabi_uldivmod(x, B8(SYS + 0x1B1));
    Q64(SYS + 0x30) = x;

    fpv = W32(SYS + 0x2BC);
    if (B8(SYS + 0x275)) {
        if ((x >> 32) == 0 && (u32)x < (fpv << 1))
            x = fpv << 1;
    } else
        x = D2ULZ((double)fpv * 1.2);
    t = x;
    B8(SYS + 0x274) = 1;
    if (B8(SYS + 0x28)) {
        Q64(SYS + 0x30) = t;
        t = W32(SYS + 0x520);
        B8(SYS + 0x274) = 0;
    } else if (B8(SYS + 0x652)) {
        B8(SYS + 0x27A) = 0;
        t = D2ULZ((double)W32(SYS + 0x2B8) * 1.2);
        Q64(SYS + 0x570) = 0;
        B8(SYS + 0x3E) = 1;
        B8(SYS + 0x102) = 1;
    }

    m = H16(SYS + 0x642);
    k = 1;
    if (m < fpv) {
        do
            k++;
        while ((long long)fpv / (long long)k > (long long)m);
    }
    Q64(SYS + 0x30) = t;
    H16(SYS + 0x644) = (unsigned short)k;
    H16(SYS + 0x640) = (unsigned short)((long long)W32(SYS + 0x2BC) / (long long)k);

    pos = S64(SYS + 0x570);
    q = pos / S64(SYS + 0x590);
    h = H16(SYS + 0xC);
    if (q > (long long)h)
        pos = (long long)(h * Q64(SYS + 0x590));
    else if (q < -(long long)h)
        pos = -(long long)(h * Q64(SYS + 0x590));
    S64(SYS + 0x570) = pos;

    switch (B8(SYS + 0x284)) {
    case 0: m = 1; break;
    case 1: m = 2; break;
    case 2: m = 3; break;
    case 3: m = 6; break;
    default: m = H16(SYS + 0x282); break;
    }
    d = B8(SYS + 0x1B1);
    iv = (int)((double)(d * (W32(SYS + 0x62C) >> 1)) * 0.95);
    W32(SYS + 0x294) = W32(SYS + 0x524) * (m * (u32)iv - (u32)iv) / d / W32(SYS + 0x540);
    H16(SYS + 0x282) = (unsigned short)m;
    W32(SYS + 0x298) = m * (u32)iv;
    H16(SYS + 0x276) = (unsigned short)(W32(SYS + 0x524) * (H16(SYS + 0x282) * (W32(SYS + 0x558) * (W32(SYS + 0x62C) >> 2) /
                                        W32(SYS + 0x540))) / B8(SYS + 0x1B1) - 1);

    pos = S64(SYS + 0x570);
    if (!B8(SYS + 0x28)) {
        h = H16(SYS + 0xC);
        p = W32(SYS + 0x294);
        lo = (int)(p - h);
        s = (int)(h - p);
        b = B8(SYS + 0x660);
        if (b >= 6) {
            double v = (double)s, pw = POW(2.0, (double)(int)(b - 6));

            v = v + pw * (B8(SYS + 0x1B0) == 2 ? -3.0 : -1.5);
            s = (int)v;
        }
        W32(SYS + 0x568) = (u32)lo;
        q = pos / S64(SYS + 0x590);
        if ((long long)s < q)
            pos = (long long)s * S64(SYS + 0x590);
        else if (q < (long long)lo)
            pos = (long long)lo * S64(SYS + 0x590);
        W32(SYS + 0x56C) = (u32)s;
    }
    S64(SYS + 0x570) = pos;

    retained_inside_current_sc1_3_t6();
    W32(SYS + 0xE0) = ratio32(0xD8, 0x50);
    W32(SYS + 0x1A0) = ratio32(0x198, 0x110);

    if (!B8(SYS + 0x28)) {
        c1 = B8(SYS + 0x46);
        c2 = B8(SYS + 0x10A);
    } else {
        c1 = B8(SYS + 0x48);
        c2 = B8(SYS + 0x10C);
    }
    B8(SYS + 0x47) = (unsigned char)c1;
    if (c1 < B8(SYS + 0x1AC))
        c1 = B8(SYS + 0x1AC);
    if (c2 < B8(SYS + 0x1AC))
        c2 = B8(SYS + 0x1AC);
    lim1 = c1 < 3 ? BW[c1] : Q64(SYS + 0x68);
    lim2 = c2 < 3 ? BW[c2] : Q64(SYS + 0x128);

    b = B8(SYS + 0x660);
    n = (b != 0 && b < 32) ? (1u << b) & 0xFFFF : 0;
    lim = B8(SYS + 0x1B0) * (W32(SYS + 0x558) * n) / W32(SYS + 0x540);
    if (B8(SYS + 0x28))
        lim = 1;
    x = __aeabi_uldivmod(Q64(SYS + 0x530), lim & 0xFFFF);
    x = __aeabi_uldivmod(x, 1000);
    u = D2ULZ(UL2D(x) / 2.24);
    if (b) {
        if (u < lim1)
            lim1 = u;
        if (u < lim2)
            lim2 = u;
    }

    if (B8(SYS + 0x3E)) {
        if (!B8(SYS + 0x102))
            B8(SYS + 0x27C) = 0;
    } else if (B8(SYS + 0x102))
        B8(SYS + 0x27C) = 1;
    W32(SYS + 0x128) = (u32)lim2;
    W32(SYS + 0x12C) = (u32)(lim2 >> 32);
    Q64(SYS + 0x68) = lim1;
    Q64(SYS + 0xF8) = u;
    H16(SYS + 0x662) = (unsigned short)lim;
    B8(SYS + 0x10B) = (unsigned char)c2;
    B8(SYS + 0x47) = (unsigned char)c1;
}
