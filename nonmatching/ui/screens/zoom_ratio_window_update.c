/*
 * zoom_ratio_window_update -- stock 0x4004201C (612 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless zoom_mode_check() is set: selects the zoom entry (zoom_entry_select), then with C = u64 SYS+0x5A0, A = s64 SYS+0x5A8, D = SYS+0x554 and P = (1e15 / C) x u16 SYS+0x1C: n = ceil(A / P) (signed); SYS+0x610 = min(n, 10 x SYS+0x5D4); q = n x D x P / A; the zoom ratio q : D is reduced to SYS+0x61C : SYS+0x618 (the smallest k with q x k divisible by D, numerator q x k / D); SYS+0x614 = q; u16 SYS+0x54A = SYS+0x610 x u16 SYS+0x5F4. Then the view position s64 SYS+0x578 is clamped so that SYS+0x578 - SYS+0x570 stays within half the plot width (u16 SYS+8) of samples either side (step s64 SYS+0x590), and the zoom window is set: u16 SYS+0x26 = (SYS+0x570 - SYS+0x578) / step + width / 2, u16 SYS+0x24 = C x width / step (at least 6). (The symbol table splits this function at 0x40042024, 0x400421A0 and 0x40042200; the entry here covers all pieces.)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"

#define SYS 0x40544E68u
#define LAY 0x42E64520u              /* UI layout table */
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define U64(a) (*(volatile unsigned long long *)(a))
#define S64(a) (*(volatile long long *)(a))

void zoom_ratio_window_update(void)
{
    unsigned long long p, k, t;
    long long a, b, c, lo, hi, d, r8;
    u32 den, n, m, q, num, wd, half;

    if (zoom_mode_check())
        return;
    zoom_entry_select();
    p = (1000000000000000ULL / U64(SYS + 0x5A0)) * H16(SYS + 0x1C);
    a = S64(SYS + 0x5A8);
    den = W32(SYS + 0x554);
    W32(SYS + 0x614) = den;
    n = (u32)(a / (long long)p);
    if (a % (long long)p != 0)
        n++;
    m = n;
    t = (unsigned long long)n * den * p;
    q = (u32)((long long)t / a);
    if (W32(SYS + 0x5D4) * 10 < n)
        m = W32(SYS + 0x5D4) * 10;
    if (q == den) {
        k = 1;
        num = 1;
    } else {
        for (k = 1; (long long)((unsigned long long)q * k) % (long long)den != 0; k++)
            ;
        num = (u32)((long long)((unsigned long long)q * k) / (long long)den);
    }
    W32(SYS + 0x61C) = num;
    W32(SYS + 0x618) = (u32)k;
    W32(SYS + 0x610) = m;
    W32(SYS + 0x614) = q;
    H16(SYS + 0x54A) = (unsigned short)(m * H16(SYS + 0x5F4));

    wd = H16(SYS + 8);
    half = wd >> 1;
    b = S64(SYS + 0x590);
    c = S64(SYS + 0x5A0);
    lo = (long long)half * (c - b);
    hi = (long long)half * (b - c);
    d = S64(SYS + 0x578) - S64(SYS + 0x570);
    r8 = S64(SYS + 0x578);
    if (d < lo)
        r8 = lo + S64(SYS + 0x570);
    else if (hi < d)
        r8 = hi + S64(SYS + 0x570);
    S64(SYS + 0x578) = r8;
    H16(SYS + 0x26) = (unsigned short)((u32)((S64(SYS + 0x570) - r8) / S64(SYS + 0x590)) + (u32)((long long)wd / 2));
    n = (u32)((U64(SYS + 0x5A0) * wd) / U64(SYS + 0x590));
    if ((n & 0xFFFF) < 6)
        n = 6;
    H16(SYS + 0x24) = (unsigned short)n;
}
