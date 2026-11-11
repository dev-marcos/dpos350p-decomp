/*
 * ldexp -- stock 0x4006B86C (240 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library ldexp, transcribed from the stock algorithm (exponent-field arithmetic,
 * with VMUL by powers of two for subnormal inputs/results). The stock subnormal-input path
 * for n >= 0x3FC scales by 2^1009 and then by 2^(n-1016); this is reproduced as is.
 */
#include "rt/bits.h"

static inline double mk(u32 hi, u32 lo) { return bitsd(((u64)hi << 32) | lo); }

double ldexp(double x, int n)
{
    u64 b = dbits(x);
    u32 hi = (u32)(b >> 32), lo = (u32)b;
    u32 e = (hi >> 20) & 0x7FF;
    u32 k = (u32)n;

    if (n >= 0) {
        if (e == 0) {
            if ((lo | (hi << 1)) == 0) return x;
            if (k < 0x3FC) return x * mk((k + 0x3FF) << 20, 0);
            x = x * mk(0x7F000000u, 0);
            k -= 0x3F8;
            b = dbits(x); hi = (u32)(b >> 32); lo = (u32)b;
            e = k + ((hi << 1) >> 21) + 1;
        } else if (e == 0x7FF) {
            return x;
        } else {
            e += k;
        }
        if (e >= 0x7FF) return mk((hi & 0x80000000u) | (0x7FFu << 20), 0);
        return mk(hi + (k << 20), lo);
    }

    k = 0u - k;
    if (e == 0) {
        if ((lo | (hi << 1)) == 0) return x;
        if (k >= 0x37) return mk(hi & 0x80000000u, 0);
    } else {
        if (e == 0x7FF) return x;
        if (k < e) return mk(hi - (k << 20), lo);
        e -= 1;
        k -= e;
        hi -= e << 20;
        x = mk(hi, lo);
        if (k >= 0x37) return mk(hi & 0x80000000u, 0);
    }
    return x * mk((0x3FF - k) << 20, 0);
}
