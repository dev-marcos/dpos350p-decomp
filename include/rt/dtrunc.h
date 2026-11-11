/* Bit-level truncation used by the stock Arm libm (pow, __sincos_core):
 * base 0x433 -> trunc(v) (integer part); base 0x434 -> integer part rounded towards zero to an
 * even integer (one more fraction bit cleared). Signs and zero are preserved. */
#ifndef RT_DTRUNC_H
#define RT_DTRUNC_H
#include "rt/bits.h"

static inline double dtrunc_base(double v, s32 base)
{
    u64 b = dbits(v);
    u32 h = (u32)(b >> 32), l = (u32)b;
    u32 h1 = (h << 1) | (l >> 31), l1 = l << 1;
    if (h1 == 0 && l1 == 0) return v;
    s32 e = base - (s32)(h1 >> 21);
    if (e >= 0x35) { h &= 0x80000000u; l = 0; }
    else if (e > 32) { h = (h >> (e - 32)) << (e - 32); l = 0; }
    else if (e == 32) { l = 0; }
    else if (e > 0) { l = (l >> e) << e; }
    return bitsd(((u64)h << 32) | l);
}

static inline int exp_all_ones(u32 hi) { return ((s32)(hi << 1) >> 21) == -1; }
static inline int inf_by_hi(u32 hi) { return exp_all_ones(hi) && (hi << 12) == 0; }
static inline int is_zero(double v) { u64 b = dbits(v); return ((u32)(b >> 32) & 0x7FFFFFFFu) == 0 && (u32)b == 0; }
static inline int nan_full(u64 b)    /* exponent all ones and any mantissa bit */
{
    u32 h = (u32)(b >> 32), l = (u32)b;
    return exp_all_ones(h) && (((h << 12) | (l >> 20)) != 0 || (l << 12) != 0);
}
#endif
