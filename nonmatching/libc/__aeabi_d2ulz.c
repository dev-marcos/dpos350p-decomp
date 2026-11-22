/*
 * __aeabi_d2ulz -- stock 0x4005F6E8 (70 bytes, Thumb)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: double (r0:r1) -> uint64, truncating. Negative values give 0; values
 * >= 2^64, +Inf and positive-sign NaN saturate to UINT64_MAX (sign taken from bit 63).
 */
#include "rt/bits.h"
#include "libc.h"

AEABI u64 __aeabi_d2ulz(u32 lo, u32 hi)
{
    s32 s = 0x41E - (s32)(hi >> 20);               /* sign bit included: negatives fall through */
    u32 m = (hi << 11) | 0x80000000u | (lo >> 21);
    if (s > 0)
        return lsr32(m, s > 127 ? 127 : (u32)s);
    s += 32;
    if (s >= 0) {
        u32 l = lsr32(lo << 11, (u32)s) + lsl32(m, 32 - (u32)s);
        return ((u64)lsr32(m, (u32)s) << 32) | l;
    }
    u32 r = ~(u32)((s32)hi >> 31);
    return ((u64)r << 32) | r;
}
