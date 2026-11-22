/*
 * llabs -- stock 0x4006A4DA (12 bytes, Thumb)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: 64-bit absolute value (llabs(LLONG_MIN) == LLONG_MIN).
 */
#include "rt/bits.h"
#include "libc.h"

s64 llabs(s64 x)
{
    return x < 0 ? (s64)(0 - (u64)x) : x;
}
