/* Exact int64 -> double conversion with round-to-nearest-even (stock 0x4003A5D8/0x4005F730). */
#ifndef RT_L2D_H
#define RT_L2D_H
#include "rt/bits.h"

static inline u64 u64_to_dbits(u64 m, u64 sign)
{
    if (m == 0) return sign;                      /* sign is 0 here (no -0 from integers) */
    u32 n = (m >> 32) ? clz32((u32)(m >> 32)) : 32 + clz32((u32)m);
    m <<= n;                                       /* leading one at bit 63 */
    u64 mant = m >> 11, rem = m & 0x7FF;
    if (rem > 0x400 || (rem == 0x400 && (mant & 1))) mant++;
    return sign | (((u64)(1085 - n) << 52) + mant);  /* mant's leading one bumps the exponent */
}
#endif
