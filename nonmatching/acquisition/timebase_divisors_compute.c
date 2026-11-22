/*
 * timebase_divisors_compute -- stock 0x4003B834 (776 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * After timebase_period_load: P = 1e15 / period SYS+0x590 x u16 SYS+0x1C; k = ceil(SYS+0x5A8 / P); the sample count SYS+0x5FC = k x W32 0x405453BC x P / SYS+0x5A8, the decimation SYS+0x5F8 = min(k, 10 x SYS+0x5D4), SYS+0x600 = the smallest j with SYS+0x5FC x j a multiple of W32 0x405453BC and SYS+0x604 the quotient; the repeat SYS+0x5F4 (u16 SYS+0x5D0, 1 in roll / with SYS+0x665) and SYS+0x544 = SYS+0x5F8 x repeat; the two channel scales SYS+0xE4 / +0x1A4 = (+0x58 / +0x118 << 10) / (+0x50 / +0x110), the spans +0xE8 / +0x1A8, the bytes SYS+0x650 / +0x651 = ceil(10 x ratio / 10) (at least 1); clears the acquisition counters SYS+0x570..+0x57F unless roll is off and SYS+0x28 set, which sets SYS+0x580.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define U64(a) (*(volatile unsigned long long *)(a))

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
