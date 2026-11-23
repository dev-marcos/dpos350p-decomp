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
#include "sys_info.h"

#define SYS 0x40544E68u
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */

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
    p = (1000000000000000ULL / U64(SYS + 0x5A0)) * SI(f_01c);
    a = S64(SYS + 0x5A8);
    den = SI(f_554);
    SI(f_614) = den;
    n = (u32)(a / (long long)p);
    if (a % (long long)p != 0)
        n++;
    m = n;
    t = (unsigned long long)n * den * p;
    q = (u32)((long long)t / a);
    if (SI(f_5d4) * 10 < n)
        m = SI(f_5d4) * 10;
    if (q == den) {
        k = 1;
        num = 1;
    } else {
        for (k = 1; (long long)((unsigned long long)q * k) % (long long)den != 0; k++)
            ;
        num = (u32)((long long)((unsigned long long)q * k) / (long long)den);
    }
    SI(f_61c) = num;
    SI(f_618) = (u32)k;
    SI(f_610) = m;
    SI(f_614) = q;
    SI(f_54a) = (unsigned short)(m * SI(f_5f4));

    wd = SI(plot_width);
    half = wd >> 1;
    b = S64(SYS + 0x590);
    c = SI(f_5a0);
    lo = (long long)half * (c - b);
    hi = (long long)half * (b - c);
    d = SI(f_578) - SI(f_570);
    r8 = SI(f_578);
    if (d < lo)
        r8 = lo + SI(f_570);
    else if (hi < d)
        r8 = hi + SI(f_570);
    SI(f_578) = r8;
    SI(f_026) = (unsigned short)((u32)((SI(f_570) - r8) / S64(SYS + 0x590)) + (u32)((long long)wd / 2));
    n = (u32)((U64(SYS + 0x5A0) * wd) / SI(f_590));
    if ((n & 0xFFFF) < 6)
        n = 6;
    SI(f_024) = (unsigned short)n;
}
