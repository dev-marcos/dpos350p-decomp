/*
 * tanh -- stock 0x4005F91C (210 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified bit-exactly by tools/emu (docs/PROGRESS.md)
 * Arm C library tanh (constants at 0x4008CC90):
 *   |x| < 2^-27 or NaN : x
 *   |x| > 19.0615      : +-1
 *   |x| >= ln2         : 1 - 2/(e^(2|x|) + 1); the stock code enters the exp kernel at
 *                        0x4006A400 with exp's own table, which for these inputs computes
 *                        exactly exp(2|x|), so this rewrite calls exp()
 *   otherwise          : rational approximation, with the half-argument identity
 *                        tanh(2y) = 2t/(1+t^2) for |x| >= ln2/2.
 */
#include "rt/bits.h"

double exp(double x);

#define BIG    bitsd(0x40330FC1931F09C9ull)   /* 19.0615 */
#define HLN2   bitsd(0x3FD62E42FEFA39EFull)
#define LN2    bitsd(0x3FE62E42FEFA39EFull)
#define A1     bitsd(0x3F91EBB5A6FA21B2ull)
#define A0     bitsd(0x3FDCB3AA0EA291F4ull)
#define B2     bitsd(0xBEDAC60B28F2D3FAull)
#define B1     bitsd(0x3F560B336C9C9114ull)
#define B0     bitsd(0x3FBD7952E534F2CCull)

double tanh(double x)
{
    u32 hi = (u32)(dbits(x) >> 32);
    u32 sign = hi & 0x80000000u;
    u32 ah = hi & ~sign;
    double ax = bitsd(dbits(x) & 0x7FFFFFFFFFFFFFFFull);
    double r;

    if ((s32)ah < 0x3E400000 || !(ax <= BIG)) {   /* tiny, huge, Inf or NaN */
        r = sign ? -ax : ax;
        if ((s32)ah < 0x3E400000) return r;
        if ((ah << 1) > 0xFFE00000u) return r;    /* NaN */
        return sign ? -1.0 : 1.0;
    }
    if (ax >= LN2) {
        double e = exp(ax + ax);
        r = e + 1.0;
        r = 2.0 / r;
        r = 1.0 - r;
        return sign ? -r : r;
    }
    int half = ax >= HLN2;
    if (half) ax = ax * 0.5;
    double z = ax * ax;
    double p = A0 + z * A1;
    double q = 1.0 + z * p;
    double b = B1 + z * B2;
    double c = B0 + z * b;
    z = z * c;
    r = ax + ax * z;
    r = r / q;
    if (half) {
        double two = r + r;
        double den = 1.0 + r * r;
        r = two / den;
    }
    return sign ? -r : r;
}
