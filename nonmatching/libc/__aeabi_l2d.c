/*
 * __aeabi_l2d -- stock 0x4003A5D8 (84 bytes, Thumb)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: int64 (r0:r1) -> double (r0:r1), round to nearest even.
 */
#include "rt/l2d.h"
#include "libc.h"

AEABI u64 __aeabi_l2d(s64 x)
{
    u64 sign = x < 0 ? 1ULL << 63 : 0;
    return u64_to_dbits(x < 0 ? 0 - (u64)x : (u64)x, sign);
}
