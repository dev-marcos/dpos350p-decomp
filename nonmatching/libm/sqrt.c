/*
 * sqrt -- stock 0x4003DF74 (30 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: VSQRT for x >= 0 (and NaN, -0). For x < 0 the stock code only replaces the
 * high word with 0x7FFFFFFF, keeping the low word of x (a NaN with x's low bits).
 */
#include "rt/bits.h"
#include "libm.h"

double sqrt(double x)
{
    if (x < 0) {
        u64 b = dbits(x);
        return bitsd((b & 0xFFFFFFFFull) | 0x7FFFFFFF00000000ull);
    }
    double r;
    __asm__("vsqrt.f64 %P0, %P1" : "=w"(r) : "w"(x));
    return r;
}
