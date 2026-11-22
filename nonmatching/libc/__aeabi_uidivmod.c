/*
 * __aeabi_uidivmod -- stock 0x4003E6F8 (12 bytes, Thumb)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: 32-bit unsigned divide, quotient in r0 and remainder in r1.
 * Stock uses UDIV + MLS, so a zero divisor yields q = 0, r = n (no trap).
 */
#include "rt/bits.h"

u64 __aeabi_uidivmod(u32 n, u32 d)
{
    u32 q = d ? n / d : 0;
    u32 r = n - d * q;
    return (u64)q | ((u64)r << 32);
}
