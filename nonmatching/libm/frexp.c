/*
 * frexp -- stock 0x40069F54 (22 bytes, Thumb, hard-float; core in 0x40069F6C)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library frexp. Zero, Inf and NaN are returned unchanged with *e = 0; subnormals are
 * normalised first. This rewrite inlines the stock core (r0:r1 -> r0:r1 + r2).
 */
#include "rt/bits.h"
#include "libm.h"

double frexp(double x, int *e)
{
    u64 b = dbits(x);
    u32 hi = (u32)(b >> 32), lo = (u32)b;
    u32 ex = (hi >> 20) & 0x7FF;
    if (ex != 0) {
        if ((hi << 1) >= 0xFFE00000u) { *e = 0; return x; }       /* Inf/NaN */
        s32 k = (s32)ex - 0x3FE;
        hi -= (u32)k << 20;
        *e = k;
        return bitsd(((u64)hi << 32) | lo);
    }
    if ((lo | (hi << 1)) == 0) { *e = 0; return x; }               /* +-0 */
    u32 sign = hi & 0x80000000u;
    u32 h = hi & ~sign;
    u32 n = clz32(h);
    if (h == 0) n += clz32(lo);
    n -= 11;
    if (n >= 32) h = lsl32(lo, n - 32);
    else h = lsl32(h, n) | lsr32(lo, 32 - n);
    h |= sign;
    lo = lsl32(lo, n);
    *e = (s32)(0u - n) - 0x3FD;
    h += 0x3FDu << 20;
    return bitsd(((u64)h << 32) | lo);
}
