/*
 * __aeabi_d2lz -- stock 0x4003DF94 (102 bytes, Thumb)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: double (r0:r1) -> int64 (r0:r1), truncating. Stock quirks reproduced:
 * |x| >= 2^63 saturates to INT64_MAX/INT64_MIN, but Inf and NaN (exponent 0x7FF) give 0.
 */
#include "rt/bits.h"

AEABI s64 __aeabi_d2lz(u32 lo, u32 hi)
{
    u32 e = (hi >> 20) & 0x7FF;
    u32 sign = (u32)((s32)hi >> 31);
    u32 m = (hi << 11) | (lo >> 21) | 0x80000000u;
    s32 s = 0x41E - (s32)e;
    if (s > 0) {                                   /* |x| < 2^31 */
        u32 r = lsr32(m, s > 127 ? 127 : (u32)s);
        if (r == 0) return 0;
        r = (r + sign) ^ sign;
        return (s64)(((u64)sign << 32) | r);
    }
    s += 32;
    if (s > 0) {                                   /* 2^31 <= |x| < 2^63 */
        u32 l = lsr32(lo << 11, (u32)s) + lsl32(m, 32 - (u32)s);
        u32 h = lsr32(m, (u32)s);
        u64 v = ((u64)h << 32) | l;
        v = (v + ((u64)sign << 32 | sign)) ^ ((u64)sign << 32 | sign);
        return (s64)v;
    }
    if (e == 0x7FF) return 0;                      /* Inf/NaN */
    return (s64)(((u64)(~sign ^ 0x80000000u) << 32) | ~sign);  /* saturate */
}
