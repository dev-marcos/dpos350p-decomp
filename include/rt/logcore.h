/* Shared argument reduction of the stock log/log10 (0x4006A31C / 0x4005DAD0).
 * Transcribed operation by operation; constants from the stock table at 0x4008C7F0. */
#ifndef RT_LOGCORE_H
#define RT_LOGCORE_H
#include "rt/bits.h"

#define LC_SQRTH  bitsd(0x3FE6A09E667F3BCDull)
#define LC_TWO52  bitsd(0x4330000000000000ull)
#define LC_C0 bitsd(0x3FC2F8D8BF29DEF7ull)
#define LC_C1 bitsd(0x3FC3991ACCFEB3F4ull)
#define LC_C2 bitsd(0x3FC7466FD968CB62ull)
#define LC_C3 bitsd(0x3FCC71C4DA12C9C6ull)
#define LC_C4 bitsd(0x3FD24924948AE94Aull)
#define LC_C5 bitsd(0x3FD9999999975FF7ull)
#define LC_C6 bitsd(0x3FE55555555555BCull)

/* Special inputs: returns 1 and sets *res when x is NaN/Inf (by high word), <= 0. */
static inline int log_special(double x, double *res)
{
    u64 b = dbits(x);
    u32 hi = (u32)(b >> 32);
    if ((hi << 1) >= 0xFFE00000u) {
        if ((hi << 1) > 0xFFE00000u || !(hi & 0x80000000u)) { *res = x; return 1; }
        *res = bitsd((b & 0xFFFFFFFFull) | 0x7FFFFFFF00000000ull);   /* -Inf -> NaN */
        return 1;
    }
    if (x <= 0.0) {
        if (x < 0.0) *res = bitsd((b & 0xFFFFFFFFull) | 0x7FFFFFFF00000000ull);
        else *res = bitsd(0xFFF0000000000000ull);                    /* log(0) = -Inf */
        return 1;
    }
    return 0;
}

/* x > 0 finite: returns f = m - 1, s*... in *pf, *pk and the value d5 = (f - z*P(z)) * s. */
static inline double log_reduce(double x, double *pf, s32 *pk)
{
    u64 b = dbits(x);
    u32 hi = (u32)(b >> 32);
    s32 k = (s32)(hi >> 20);
    if (k == 0) {                                  /* subnormal: scale by 2^52 */
        x = x * LC_TWO52;
        b = dbits(x);
        hi = (u32)(b >> 32);
        k = (s32)(hi >> 20) - 0x3FE;
        hi -= (u32)k << 20;
        k -= 0x34;
    } else {
        k -= 0x3FE;
        hi -= (u32)k << 20;
    }
    double m = bitsd(((u64)hi << 32) | (u32)dbits(x));
    if (m < LC_SQRTH) { k -= 1; m = m * 2.0; }
    double f = m - 1.0;
    double den = m + 1.0;
    double s = f / den;
    double z = s * s;
    double a = LC_C1 + LC_C0 * z;
    double c = LC_C2 + a * z;
    double d = LC_C3 + c * z;
    double e = LC_C4 + d * z;
    double g = LC_C5 + e * z;
    double h = LC_C6 + g * z;
    double r = f - h * z;
    *pf = f;
    *pk = k;
    return r * s;
}
#endif
