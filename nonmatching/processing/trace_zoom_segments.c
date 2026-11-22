/*
 * trace_zoom_segments -- stock 0x4003C314 (524 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When the zoom factor k = SYS+0x554 is below the samples per step (mode 0/1: +0x5FC, else +0x614): rem = ((base - SYS+0x5E0) x per-sample (+0x5F8 / +0x610, signed) / div) %% step (bases and divisors as in trace_shift_samples), the view offsets become rem + offset x samples / k, and each segment k of trace tr is stretched with index_map_if_above(buffer, SYS+0x38C[k], u16 SYS+0x454[k], samples, rem as a 64-bit stack argument). mode_not_0_or_2(tr) is called and its result ignored (stock).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

u32 mode_not_0_or_1(u32 v);
#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define I32(a) (*(volatile int *)(a))
#define S64(a) (*(volatile long long *)(a))
#define P(o) (SYS + 0x5F4 + (o))
#define TRBUF(ch) (0x411AF3E8u + (ch) * 0x927C0u)

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
