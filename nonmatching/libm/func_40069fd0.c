/*
 * __pow_core -- stock 0x40069FD0 (842 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified bit-exactly by tools/emu (docs/PROGRESS.md)
 * Arm C library internal of pow: returns ldexp(y * e^x, k) (d0 = x, d1 = y, r0 = k), with the
 * same reduction/rational approximation as exp (constants built inline) and a special-case
 * chain for zero/Inf/NaN operands, transcribed from the stock code (compiled C).
 */
#include "rt/dtrunc.h"

double ldexp(double x, int n);

#define K(x) bitsd(x##ull)

static double finish(double r) { return r; }   /* stock post-checks only set flags */

double __pow_core(double x, double y, int k)
{
    u64 xb = dbits(x), yb = dbits(y);
    u32 hx = (u32)(xb >> 32), hy = (u32)(yb >> 32);

    if (exp_all_ones(hx) || is_zero(x) || exp_all_ones(hy) || is_zero(y)) {
        if (exp_all_ones(hx) && nan_full(xb)) return x;            /* x NaN */
        if (exp_all_ones(hy) && nan_full(yb)) return y;            /* y NaN */
        if (is_zero(y)) {
            if (!exp_all_ones(hx) || (hx << 12) != 0) return y;
            return K(0x7FFFFFFFFFFFFFFF);                          /* 0 * Inf */
        }
        if (exp_all_ones(hy) && (hy << 12) == 0) {                 /* y Inf (by high word) */
            if (!exp_all_ones(hx) || (hx << 12) != 0) return y;
            return K(0x7FFFFFFFFFFFFFFF);
        }
        if (is_zero(x))
            return finish(ldexp(y, k));                            /* e^0 = 1 */
        if (exp_all_ones(hx) && (hx << 12) == 0) return x * y;     /* x Inf */
        return bitsd((u64)((u32)((s32)((hx << 1) | ((u32)xb >> 31)) >> 31) & 0x80000000u) << 32);
    }

    if (!(x < K(0xC09CC80000000000)))                              /* x >= -1842 */
        goto not_low;
    return bitsd((u64)(hy & 0x80000000u) << 32);                  /* +-0 */
not_low:
    if (!(x < K(0x409CC80000000001)))                              /* x >= 1842 */
        return bitsd((u64)((hy & 0x80000000u) | 0x7FF00000u) << 32);   /* +-Inf */

    double t = x * K(0x3FF71547652B82FE);
    double half = bitsd((u64)(((u32)(dbits(t) >> 32) & 0x80000000u) | 0x3FE00000u) << 32);
    t = half + t;
    s32 n = (s32)t;
    double nd = (double)n;
    double r = x + nd * K(0xBFE62E42F8000000);
    r = r + nd * K(0xBE4BE8E7BCD5E4F2);
    double ar = bitsd(dbits(r) & 0x7FFFFFFFFFFFFFFFull);
    if (!(ar < K(0x3C90000000000000))) {                           /* |r| >= 2^-54 */
        double z = r * r;
        double p = K(0x3FBC718F8C124358) + z * K(0x3F503FA081572E11);
        double q = K(0x3F8C70E8DAF3BD0B) + z * K(0x3F0152B741A5E84B);
        double pz = p * z;
        double hq = 0.5 + q * z;
        hq = hq * r;
        double num = pz + hq;
        double den = pz - hq;
        num = num + 1.0;
        den = den + 1.0;
        y = (num / den) * y;
    }
    return finish(ldexp(y, n + k));
}
