/*
 * timebase_divisors_compute -- stock 0x4003B834 (776 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * After timebase_period_load: P = 1e15 / period SYS+0x590 x u16 SYS+0x1C; k = ceil(SYS+0x5A8 / P); the sample count SYS+0x5FC = k x W32 0x405453BC x P / SYS+0x5A8, the decimation SYS+0x5F8 = min(k, 10 x SYS+0x5D4), SYS+0x600 = the smallest j with SYS+0x5FC x j a multiple of W32 0x405453BC and SYS+0x604 the quotient; the repeat SYS+0x5F4 (u16 SYS+0x5D0, 1 in roll / with SYS+0x665) and SYS+0x544 = SYS+0x5F8 x repeat; the two channel scales SYS+0xE4 / +0x1A4 = (+0x58 / +0x118 << 10) / (+0x50 / +0x110), the spans +0xE8 / +0x1A8, the bytes SYS+0x650 / +0x651 = ceil(10 x ratio / 10) (at least 1); clears the acquisition counters SYS+0x570..+0x57F unless roll is off and SYS+0x28 set, which sets SYS+0x580.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))

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
    n = SI(f_01c);
    p = (1000000000000000ULL / SI(f_590)) * n;
    d = W32(0x405453BCu);
    SI(f_5fc) = d;
    a = SI(f_5a8);
    k = (u32)(a / p);
    if (a % p)
        k++;
    sl = (u32)((long long)((unsigned long long)k * d * p) / (long long)a);
    c = SI(f_5d4) * 10;
    SI(f_5f8) = c < k ? c : k;
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
    SI(f_604) = q;
    SI(f_5fc) = sl;
    SI(f_600) = (u32)j;
    h = SI(f_5d0);
    if (SI(acq_transition_flag) || SI(f_665))
        h = 1;
    SI(f_5f4) = (unsigned short)h;
    SI(f_544) = (unsigned short)(SI(f_5f8) * h);
    SI(f_0e4) = (u32)((long long)((unsigned long long)SI(f_058) << 10) / S64(SYS + 0x50));
    SI(f_0e8) = SI(f_064) - SI(ch1_offset);
    SI(f_1a4) = (u32)((long long)((unsigned long long)SI(f_118) << 10) / S64(SYS + 0x110));
    SI(f_1a8) = SI(f_124) - SI(ch2_offset);
    v = (long long)((unsigned long long)SI(f_058) * 10) / S64(SYS + 0x50);
    g1 = ceil_div10(v);
    v = (long long)((unsigned long long)SI(f_118) * 10) / S64(SYS + 0x110);
    g2 = ceil_div10(v);
    SI(f_650) = (unsigned char)g1;
    SI(f_651) = (unsigned char)g2;
    if ((unsigned char)g1 == 0)
        SI(f_650) = 1;
    if ((unsigned char)g2 == 0)
        SI(f_651) = 1;
    if (SI(acq_path_mode) && !SI(acq_transition_flag)) {
        SI(f_580) = 0;
        W32(SYS + 0x570) = 0;
        W32(SYS + 0x574) = 0;
        SI(f_578) = 0;
    } else {
        SI(f_580) = 1;
    }
}
