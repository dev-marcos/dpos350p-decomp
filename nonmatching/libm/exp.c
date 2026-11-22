/*
 * exp -- stock 0x40068F08 (92 bytes, Thumb, hard-float; kernel at 0x4006A3F8..0x4006A4D8)
 * Status: NONMATCHING; behaviour verified bit-exactly by tools/emu (docs/PROGRESS.md)
 * Arm C library exp, transcribed operation by operation (VMLA/VMLS are not fused: build with
 * -ffp-contract=off). Constants are the stock table at 0x4008C898. The shared kernel is
 * inlined here; its register-interface entry used by tanh (0x4006A400) stays in asm/.
 * Stock quirks kept: special cases are decided on the high word only (a NaN whose high word
 * equals -Inf's returns +0); |r| < 2^-54 skips the rational approximation.
 */
#include "rt/bits.h"
#include "libm.h"

#define LOG2E   bitsd(0x3FF71547652B82FEull)   /* 1/ln2 */
#define LN2HI   bitsd(0x3FE62E42F8000000ull)
#define LN2LO   bitsd(0x3E4BE8E7BCD5E4F2ull)
#define P1      bitsd(0x3F503FA081572E11ull)
#define P0      bitsd(0x3FBC718F8C124358ull)
#define Q2      bitsd(0x3F0152B741A5E84Bull)
#define Q1      bitsd(0x3F8C70E8DAF3BD0Bull)
#define TWO1023 bitsd(0x7FE0000000000000ull)
#define TWOM1022 bitsd(0x0010000000000000ull)
#define INF     bitsd(0x7FF0000000000000ull)

static inline double pow2field(u32 f) { return bitsd((u64)(f << 20) << 32); }

double exp(double x)
{
    u32 hi = (u32)(dbits(x) >> 32);
    u32 h2 = hi << 1;

    if (h2 >= 0x810E9222u) {                       /* |x| >= ~709.78, Inf or NaN */
        if (h2 >= 0xFFE00000u) {
            if (h2 > 0xFFE00000u) return x;            /* NaN (by high word) */
            if (hi & 0x80000000u) return 1.0 - 1.0;     /* -Inf -> +0 */
            return x;                                   /* +Inf */
        }
        return (hi & 0x80000000u) ? 1.0 - 1.0 : INF;    /* underflow / overflow */
    }
    if (x == 0.0) return 1.0;

    double t = (hi & 0x80000000u) ? -0.5 : 0.5;
    t = t + x * LOG2E;
    s32 k = (s32)t;                                /* VCVT.S32.F64: truncate */
    double kd = (double)k;
    double r = x - kd * LN2HI;
    r = r - kd * LN2LO;
    u32 rh2 = (u32)(dbits(r) >> 32) << 1;
    double z = r * r;
    double p = P0 + z * P1;
    double scale = 1.0;

    if (rh2 >= 0x79200000u) {                      /* |r| >= 2^-54 */
        double q = Q1 + z * Q2;
        p = p * z;
        q = 0.5 + z * q;
        double num = p + r * q;
        num = num + 1.0;
        double den = p - r * q;
        den = den + 1.0;
        scale = scale * (num / den);
    }

    s32 n = k + 0x400;
    if (n > 1 && 0x800 - n > 0)
        return pow2field((u32)(n - 1)) * scale;
    s32 m = n - 1;
    if (m >= 1) {                                  /* result overflows the exponent field */
        m -= 0x3FE;
        if ((u32)m >= 0x800) return INF * scale;
        m -= 1;
        scale = scale * TWO1023;
        return pow2field((u32)m) * scale;
    }
    m += 0x3FE;                                    /* subnormal result */
    if (m < 1) return (scale - scale) * scale;
    scale = scale * TWOM1022;
    return pow2field((u32)m) * scale;
}
