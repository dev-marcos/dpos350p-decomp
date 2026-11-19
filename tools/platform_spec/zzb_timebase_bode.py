# The 1-2-5 step table of the time base, the divisor set-up that follows a time-base change, and
# the zoom of the gain / phase curves of the frequency-response (Bode) view.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="timebase"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define U64(a) (*(volatile unsigned long long *)(a))
"""

wr(0x4003A680, 572, "timebase_step_table_init",
   "Fills the 36 s64 entries at SYS+0xBD0 with the 1-2-5 series 1, 2, 5, 10 .. 5e13 (the time-base steps read by timebase_period_load). "
   "Entry 11 (SYS+0xC28) is 5000000 instead of 5000 in this build.",
   COM + """
void timebase_step_table_init(void)
{
    S64(SYS + 0xBD0) = 1LL;
    S64(SYS + 0xBD8) = 2LL;
    S64(SYS + 0xBE0) = 5LL;
    S64(SYS + 0xBE8) = 10LL;
    S64(SYS + 0xBF0) = 20LL;
    S64(SYS + 0xBF8) = 50LL;
    S64(SYS + 0xC00) = 100LL;
    S64(SYS + 0xC08) = 200LL;
    S64(SYS + 0xC10) = 500LL;
    S64(SYS + 0xC18) = 1000LL;
    S64(SYS + 0xC20) = 2000LL;
    S64(SYS + 0xC28) = 5000000LL;      /* sic: the series would give 5000 */
    S64(SYS + 0xC30) = 5000LL;
    S64(SYS + 0xC38) = 10000LL;
    S64(SYS + 0xC40) = 20000LL;
    S64(SYS + 0xC48) = 50000LL;
    S64(SYS + 0xC50) = 100000LL;
    S64(SYS + 0xC58) = 200000LL;
    S64(SYS + 0xC60) = 500000LL;
    S64(SYS + 0xC68) = 1000000LL;
    S64(SYS + 0xC70) = 2000000LL;
    S64(SYS + 0xC78) = 5000000LL;
    S64(SYS + 0xC80) = 10000000LL;
    S64(SYS + 0xC88) = 20000000LL;
    S64(SYS + 0xC90) = 50000000LL;
    S64(SYS + 0xC98) = 100000000LL;
    S64(SYS + 0xCA0) = 200000000LL;
    S64(SYS + 0xCA8) = 500000000LL;
    S64(SYS + 0xCB0) = 1000000000LL;
    S64(SYS + 0xCB8) = 2000000000LL;
    S64(SYS + 0xCC0) = 5000000000LL;
    S64(SYS + 0xCC8) = 10000000000LL;
    S64(SYS + 0xCD0) = 20000000000LL;
    S64(SYS + 0xCD8) = 50000000000LL;
    S64(SYS + 0xCE0) = 100000000000LL;
    S64(SYS + 0xCE8) = 200000000000LL;
    S64(SYS + 0xCF0) = 500000000000LL;
    S64(SYS + 0xCF8) = 1000000000000LL;
    S64(SYS + 0xD00) = 2000000000000LL;
    S64(SYS + 0xD08) = 5000000000000LL;
    S64(SYS + 0xD10) = 10000000000000LL;
    S64(SYS + 0xD18) = 20000000000000LL;
    S64(SYS + 0xD20) = 50000000000000LL;
}
""")

wr(0x4003B834, 776, "timebase_divisors_compute",
   "After timebase_period_load: P = 1e15 / period SYS+0x590 x u16 SYS+0x1C; k = ceil(SYS+0x5A8 / P); the sample count SYS+0x5FC = k x W32 0x405453BC x P / SYS+0x5A8, the decimation SYS+0x5F8 = min(k, 10 x SYS+0x5D4), "
   "SYS+0x600 = the smallest j with SYS+0x5FC x j a multiple of W32 0x405453BC and SYS+0x604 the quotient; the repeat SYS+0x5F4 (u16 SYS+0x5D0, 1 in roll / with SYS+0x665) and SYS+0x544 = SYS+0x5F8 x repeat; "
   "the two channel scales SYS+0xE4 / +0x1A4 = (+0x58 / +0x118 << 10) / (+0x50 / +0x110), the spans +0xE8 / +0x1A8, the bytes SYS+0x650 / +0x651 = ceil(10 x ratio / 10) (at least 1); "
   "clears the acquisition counters SYS+0x570..+0x57F unless roll is off and SYS+0x28 set, which sets SYS+0x580.",
   COM + """
void timebase_period_load(void);

static u32 ceil_div10(long long v)
{
    u32 a = (u32)(v / 10);
    if (v % 10)
        a++;
    return a;
}

void timebase_divisors_compute(void)
{
    unsigned long long p, a, pr, j;
    u32 n, d, k, c, sl, q, h, g1, g2;
    long long v;
    timebase_period_load();
    n = H16(SYS + 0x1C);
    p = (1000000000000000ULL / U64(SYS + 0x590)) * n;
    d = W32(0x405453BCu);
    W32(SYS + 0x5FC) = d;
    a = U64(SYS + 0x5A8);
    k = (u32)(a / p);
    if (a % p)
        k++;
    sl = (u32)((long long)((unsigned long long)k * d * p) / (long long)a);
    c = W32(SYS + 0x5D4) * 10;
    W32(SYS + 0x5F8) = c < k ? c : k;
    j = 1;
    if (sl == d) {
        q = 1;
    } else {
        for (;;) {
            pr = (unsigned long long)sl * j;
            if ((long long)pr % (long long)(unsigned long long)W32(0x405453BCu) == 0)
                break;
            j++;
        }
        q = (u32)((long long)pr / (long long)(unsigned long long)W32(0x405453BCu));
    }
    W32(SYS + 0x604) = q;
    W32(SYS + 0x5FC) = sl;
    W32(SYS + 0x600) = (u32)j;
    h = H16(SYS + 0x5D0);
    if (B8(SYS + 0x3C) || B8(SYS + 0x665))
        h = 1;
    H16(SYS + 0x5F4) = (unsigned short)h;
    H16(SYS + 0x544) = (unsigned short)(W32(SYS + 0x5F8) * h);
    W32(SYS + 0xE4) = (u32)((long long)((unsigned long long)S64(SYS + 0x58) << 10) / S64(SYS + 0x50));
    W32(SYS + 0xE8) = W32(SYS + 0x64) - W32(SYS + 0x60);
    W32(SYS + 0x1A4) = (u32)((long long)((unsigned long long)S64(SYS + 0x118) << 10) / S64(SYS + 0x110));
    W32(SYS + 0x1A8) = W32(SYS + 0x124) - W32(SYS + 0x120);
    v = (long long)((unsigned long long)S64(SYS + 0x58) * 10) / S64(SYS + 0x50);
    g1 = ceil_div10(v);
    v = (long long)((unsigned long long)S64(SYS + 0x118) * 10) / S64(SYS + 0x110);
    g2 = ceil_div10(v);
    B8(SYS + 0x650) = (unsigned char)g1;
    B8(SYS + 0x651) = (unsigned char)g2;
    if ((unsigned char)g1 == 0)
        B8(SYS + 0x650) = 1;
    if ((unsigned char)g2 == 0)
        B8(SYS + 0x651) = 1;
    if (B8(SYS + 0x28) && !B8(SYS + 0x3C)) {
        B8(SYS + 0x580) = 0;
        W32(SYS + 0x570) = 0;
        W32(SYS + 0x574) = 0;
        S64(SYS + 0x578) = 0;
    } else {
        B8(SYS + 0x580) = 1;
    }
}
""")

wr(0x40039918, 744, "bode_curves_zoom",
   "Unless touch byte 0x42E68BB0+0x17 is set, zooms the gain and the phase curve of the Bode view (u16 rows 0x42E180C8+0x8D20 / +0x9CC0, W32 +0x5C points) to u16 +4 pixels: "
   "each curve is centred at index 10000 of the workspace 0x416431E8 with 100 copies of its end samples, resampled (m = +4 x SYS+0x554 / +0x5C, kept in +0x40; nearest neighbour below 3 x SYS+0x554, "
   "else spline) and copied back; then every row is clamped to 3 .. u16 +6 - 3 and flipped to +6 - row.",
   COM + """
#define BB 0x42E180C8u
#define WS 0x416431E8u
#define MID (WS + 0x4E20u)

void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
void fill_u16(unsigned short *d, unsigned short v, u32 n);
void simple_integer_index_mapping_nearest_neighbour(u32 src_a, u32 center_a, u32 w, u32 m, u32 shift_a, u32 unused);
void floating_point_interpolation_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift);

static void zoom_one(u32 off)
{
    int c = (int)W32(BB + 0x5C);
    u32 n;
    copy_u16((unsigned short *)(MID - 2 * (u32)(c / 2)), (const unsigned short *)(BB + off), (u32)c + 100);
    c = (int)W32(BB + 0x5C);
    fill_u16((unsigned short *)(MID - 200 - 2 * (u32)(c / 2)), H16(BB + off), 100);
    c = (int)W32(BB + 0x5C);
    fill_u16((unsigned short *)(MID + 2 * (u32)(c / 2)), H16(BB + 2 * (u32)c + off - 2), 100);
    if (W32(BB + 0x40) < 3 * W32(SYS + 0x554))
        simple_integer_index_mapping_nearest_neighbour(WS, 10000, H16(BB + 4), W32(BB + 0x40), 0, 0);
    else
        floating_point_interpolation_resampling(WS, 10000, H16(BB + 4), W32(BB + 0x40), 0LL);
    n = H16(BB + 4);
    copy_u16((unsigned short *)(BB + off), (const unsigned short *)(MID - (n & ~1u)), n);
}

static void clamp_flip(u32 off)
{
    volatile unsigned short *p = (volatile unsigned short *)(BB + off);
    u32 i, h;
    if (H16(BB + 4) == 0)
        return;
    i = 0;
    do {
        if ((short)*p < 3) {
            *p = 3;
        } else {
            h = H16(BB + 6);
            if ((int)(short)h - 3 < (int)(short)*p)
                *p = (unsigned short)(0xFFFD + h);
        }
        *p = (unsigned short)(H16(BB + 6) - *p);
        p++;
        i++;
    } while (i < H16(BB + 4));
}

void bode_curves_zoom(void)
{
    if (B8(0x42E68BB0u + 0x17))
        return;
    W32(BB + 0x40) = H16(BB + 4) * W32(SYS + 0x554) / W32(BB + 0x5C);
    zoom_one(0x8D20);
    zoom_one(0x9CC0);
    clamp_flip(0x8D20);
    clamp_flip(0x9CC0);
}
""", src="trace")
