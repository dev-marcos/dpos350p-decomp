/*
 * log10 -- stock 0x4005DAD0 (202 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified bit-exactly by tools/emu (docs/PROGRESS.md)
 * Arm C library base-10 logarithm: same reduction as log, then recombination with split
 * constants (1/ln10 and log10(2) in hi/lo parts). Special cases shared with log.
 */
#include "rt/logcore.h"

#define K0 bitsd(0x3E7427DE7FBCC47Cull)   /* log10(2) lo */
#define K1 bitsd(0x3FDBCB7B1526E50Eull)   /* 1/ln10 */
#define K2 bitsd(0x3E5526E50E32A6ABull)   /* 1/ln10 lo */
#define K3 bitsd(0x3FDBCB7B00000000ull)   /* 1/ln10 hi */
#define K4 bitsd(0x3FD3441300000000ull)   /* log10(2) hi */

double log10(double x)
{
    double res, f;
    s32 k;
    if (log_special(x, &res)) return res;
    double t = log_reduce(x, &f, &k);
    double kd = (double)k;
    double r = K0 * kd;
    r = r - K1 * t;
    r = r + K2 * f;
    r = r + K3 * f;
    return r + K4 * kd;
}
