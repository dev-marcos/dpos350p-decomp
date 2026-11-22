/*
 * __aeabi_f2lz -- stock 0x4006A4E8 (72 bytes, Thumb)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: float (r0) -> int64 (r0:r1), truncating; saturates on overflow, Inf/NaN -> 0.
 */
#include "rt/bits.h"

AEABI s64 __aeabi_f2lz(u32 x)
{
    u32 e = (x >> 23) & 0xFF;
    u32 sign = (u32)((s32)x >> 31);
    u32 m = (x << 8) | 0x80000000u;
    s32 s = 0x9E - (s32)e;
    if (s > 0) {
        u32 r = lsr32(m, (u32)s & 0xFF);
        if (r == 0) return 0;
        return (s64)(((u64)sign << 32) | ((r + sign) ^ sign));
    }
    s += 32;
    if (s > 0) {
        u32 h = lsr32(m, (u32)s) ^ sign;
        u32 l = lsl32(m, 32 - (u32)s) ^ sign;
        u64 v = (((u64)h << 32) | l) - (((u64)sign << 32) | sign);
        return (s64)v;
    }
    if (e == 0xFF) return 0;
    return (s64)(((u64)(~sign ^ 0x80000000u) << 32) | ~sign);
}
