/*
 * __sincos_core -- stock 0x4006613C (1012 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified bit-exactly by tools/emu (docs/PROGRESS.md)
 * Arm C library shared kernel of sin (q = 0, 0x40066534) and cos (q = 1, 0x40066530).
 * Transcribed from the stock code (compiled C; constants built with MOVW/MOVT):
 *   n = trunc(x*2/pi +- 0.5); r = x + (n*2^-28)*C1 + ... + (n*2^-28)*C5  (-pi/2*2^28 in 5 parts)
 *   for |x*2/pi| >= 2147483647.5 n is truncated by bit manipulation; Inf -> NaN, NaN -> x.
 *   q += n & 3; odd q -> cosine polynomial, bit 1 of q -> negate; |r| < 2^-27 short-cuts.
 */
#include "rt/bits.h"

#define K(x) bitsd(x##ull)
#define TWO_OVER_PI K(0x3FE45F306DC9C883)
#define THRESH      K(0x41DFFFFFFFA00000)
#define C1 K(0xC1B921FB50000000)
#define C2 K(0xC01110B460000000)
#define C3 K(0xBE51A62620000000)
#define C4 K(0xBCD3145C00000000)
#define C5 K(0xBB3B839A20000000)

static inline double with_sign_of(double v, u32 hi_src)
{
    return bitsd(((u64)((hi_src & 0x80000000u) | (u32)(dbits(v) >> 32)) << 32) | (u32)dbits(v));
}

static inline double reduce(double r, double f)
{
    r = r + f * C1;
    r = r + f * C2;
    r = r + f * C3;
    r = r + f * C4;
    r = r + f * C5;
    return r;
}

double __sincos_core(double x, int q)
{
    double t = x * TWO_OVER_PI;
    u32 th = (u32)(dbits(t) >> 32);
    double at = bitsd(dbits(t) & 0x7FFFFFFFFFFFFFFFull);
    double r = x;
    s32 n;

    if (at < THRESH) {
        t = with_sign_of(0.5, th) + t;
        n = (s32)t;                                       /* VCVT.S32.F64 (truncate) */
        if (n != 0)
            r = reduce(r, (double)n * K(0x3E30000000000000));   /* VCVT.F64.S32 #28 = n*2^-28 */
    } else {
        u64 xb = dbits(x);
        u32 hi = (u32)(xb >> 32), lo = (u32)xb;
        u32 top = (hi << 1) | (lo >> 31);
        if (((s32)top >> 21) == -1) {                      /* exponent 0x7FF */
            if (((hi << 12) | (lo >> 20)) == 0 && (lo << 12) == 0)
                return bitsd(0x7FFFFFFFFFFFFFFFull);       /* sin/cos(Inf) = NaN */
            return x;                                      /* NaN */
        }
        if ((hi & 0x7FFFFFFFu) == 0 && lo == 0)            /* x == 0 (unreachable here) */
            return q == 0 ? x : 1.0;
        t = with_sign_of(0.5, th) + t;
        u64 tb = dbits(t);
        u32 h = (u32)(tb >> 32), l = (u32)tb;
        u32 h1 = (h << 1) | (l >> 31), l1 = l << 1;
        if (h1 != 0 || l1 != 0) {                          /* truncate t to an integer */
            s32 e = 0x433 - (s32)(h1 >> 21);               /* number of fraction bits */
            if (e >= 0x35) { h &= 0x80000000u; l = 0; }
            else if (e > 32) { h = (h >> (e - 32)) << (e - 32); l = 0; }
            else if (e == 32) { l = 0; }
            else if (e > 0) { l = (l >> e) << e; }
            t = bitsd(((u64)h << 32) | l);
        }
        tb = dbits(t);
        if ((u32)(tb >> 32) & 0x7FFFFFFFu || (u32)tb)
            r = reduce(r, t * K(0x3E30000000000000));
        n = (s32)t;                                        /* saturating VCVT; only n & 3 used */
    }

    q = (n & 3) + q;
    double ar = bitsd(dbits(r) & 0x7FFFFFFFFFFFFFFFull);
    if (ar < K(0x3E40000000000000)) {                      /* |r| < 2^-27 */
        if (q & 1) r = 1.0;
    } else if (q & 1) {                                    /* cosine polynomial */
        double z = r * r;
        double c = K(0xBE927DF9C661A83B) + z * K(0x3E21BA39100A2609);
        c = K(0x3EFA019F9030AC63) + c * z;
        c = K(0xBF56C16C1651FBF6) + c * z;
        c = K(0x3FA55555555502B1) + c * z;
        c = K(0xBFDFFFFFFFFFFFAE) + c * z;
        r = 1.0 + c * z;
    } else {                                               /* sine polynomial */
        double z = r * r;
        double s = K(0xBE5AE5E1A1730C8F) + z * K(0x3DE5D810C32F8FB0);
        s = K(0x3EC71DE34FE4AD4D) + s * z;
        s = K(0xBF2A01A019B5E33E) + s * z;
        s = K(0x3F8111111110F0FA) + s * z;
        double rz = r * z;
        s = K(0xBFC5555555555542) + s * z;
        r = r + rz * s;
    }
    if (q & 2) r = -r;
    return r;
}
