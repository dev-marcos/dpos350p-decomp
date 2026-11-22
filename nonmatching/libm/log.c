/*
 * log -- stock 0x4006A31C (218 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified bit-exactly by tools/emu (docs/PROGRESS.md)
 * Arm C library natural logarithm: m in [sqrt(1/2), sqrt(2)), s = (m-1)/(m+1), odd series in s,
 * result = f - (s*(f - z*P) - k*ln2lo) + k*ln2hi. Stock special cases: log(0) = -Inf,
 * x < 0 or -Inf -> NaN built from x's low word (high word 0x7FFFFFFF).
 */
#include "rt/logcore.h"
#include "libm.h"

#define LN2LO bitsd(0x3E7F7D1CF79ABC9Eull)
#define LN2HI bitsd(0x3FE62E42C0000000ull)

double log(double x)
{
    double res, f;
    s32 k;
    if (log_special(x, &res)) return res;
    double t = log_reduce(x, &f, &k);
    double kd = (double)k;
    t = t - LN2LO * kd;
    double r = f - t;
    return r + kd * LN2HI;
}
