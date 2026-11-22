/*
 * pow -- stock 0x4005D4A4 (1580 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified bit-exactly by tools/emu (docs/PROGRESS.md)
 * Arm C library pow, transcribed from the stock code (compiled C):
 *   m = frexp(|x|, &e); c = 0.5 + k/64 nearest table point (k = trunc(0.5 + 64m) - 32), or
 *   c = 1, m *= 2 when e == 1 and m < sqrt(1/2); s = (m-c)/(m+c); log2|x| = e + T0[k] +
 *   log2e * ln-series(s); t = y*log2|x| clamped to (-32768, 32768); n = round(t);
 *   result = __pow_core(residual*ln2, 1.0, n), negated for negative x with odd integer y.
 * Table T (log2 c in three parts) is the stock table at 0x40084A10 (33 entries of 32 bytes).
 * Stock special-case quirks are reproduced, e.g. (-Inf)^(+-Inf) is treated as an odd power.
 */
#include "rt/dtrunc.h"
#include "libm.h"

#define K(x) bitsd(x##ull)

static const u64 T[33][3] = {
    {0xBFF0000000000000ull, 0x8000000000000000ull, 0x8000000000000000ull},
    {0xBFEE9452C0000000ull, 0xBE514E2048000000ull, 0xBCBC6764F8000000ull},
    {0xBFED338120000000ull, 0xBE14DBB380000000ull, 0xBCA3066750000000ull},
    {0xBFEBDCE9D0000000ull, 0xBE5992C308000000ull, 0xBCB8D3B080000000ull},
    {0xBFEA8FF970000000ull, 0xBE2810A5C0000000ull, 0xBCB0C0BFE8000000ull},
    {0xBFE94C2870000000ull, 0xBE424B1360000000ull, 0xBCB8203DC0000000ull},
    {0xBFE810FA50000000ull, 0xBE2BF65FC0000000ull, 0xBCA7715C90000000ull},
    {0xBFE6DDFC20000000ull, 0xBE54F1F8C0000000ull, 0xBCB9F6E918000000ull},
    {0xBFE5B2C3D0000000ull, 0xBE5432E470000000ull, 0xBCB406DB50000000ull},
    {0xBFE48EEF10000000ull, 0xBE5262F320000000ull, 0xBC9A872AC0000000ull},
    {0xBFE37222B0000000ull, 0xBE56E0E8F0000000ull, 0xBCBD2D3528000000ull},
    {0xBFE25C0A00000000ull, 0xBE418EFAB0000000ull, 0xBCBD6FAE40000000ull},
    {0xBFE14C5600000000ull, 0xBE5FCD15F0000000ull, 0xBC901C1400000000ull},
    {0xBFE042BD40000000ull, 0xBE5734F930000000ull, 0xBC931E6CE0000000ull},
    {0xBFDE7DF5E0000000ull, 0xBE5E538AB0000000ull, 0xBCAA77DA80000000ull},
    {0xBFDC819DC0000000ull, 0xBE26A2FF00000000ull, 0xBCB0E25760000000ull},
    {0xBFDA8FF960000000ull, 0xBE51810A58000000ull, 0xBCB8605FF0000000ull},
    {0xBFD8A89800000000ull, 0xBE457F7A60000000ull, 0xBCA33354D0000000ull},
    {0xBFD6CB0F60000000ull, 0xBE40CB91D0000000ull, 0xBCA01B6D40000000ull},
    {0xBFD4F6FBA0000000ull, 0xBE52CEC598000000ull, 0xBC7E393A00000000ull},
    {0xBFD32BFEE0000000ull, 0xBE2B877340000000ull, 0xBC79689200000000ull},
    {0xBFD169C040000000ull, 0xBE5363F158000000ull, 0xBC7C8D4380000000ull},
    {0xBFCF5FD880000000ull, 0xBE54831F18000000ull, 0xBCA2423FB0000000ull},
    {0xBFCBFC6780000000ull, 0xBE53FFFA60000000ull, 0xBCB80DE050000000ull},
    {0xBFC8A89800000000ull, 0xBE357F7A60000000ull, 0xBC933354C0000000ull},
    {0xBFC563DC00000000ull, 0xBE54FFD658000000ull, 0xBC90643900000000ull},
    {0xBFC22DADC0000000ull, 0xBE1559A480000000ull, 0xBCAB4B48E0000000ull},
    {0xBFBE0B1A80000000ull, 0xBE5A3CBF50000000ull, 0xBCB664B3A0000000ull},
    {0xBFB7D60480000000ull, 0xBE36CFBB40000000ull, 0xBCA8CE7680000000ull},
    {0xBFB1BB3280000000ull, 0xBE43002A40000000ull, 0xBCBD0CC628000000ull},
    {0xBFA7739400000000ull, 0xBE593B2B18000000ull, 0xBCA55DE5C0000000ull},
    {0xBF9743EE00000000ull, 0xBE40C3E6A0000000ull, 0xBCB58D9520000000ull},
    {0x8000000000000000ull, 0x8000000000000000ull, 0x8000000000000000ull},
};

static double special(double x, double y)
{
    u64 xb = dbits(x), yb = dbits(y);
    u32 hx = (u32)(xb >> 32), hy = (u32)(yb >> 32);
    int r0;

    if (exp_all_ones(hx) && nan_full(xb)) return x;
    if (exp_all_ones(hy) && nan_full(yb)) return y;
    r0 = 1;                                        /* y = +-Inf keeps r0 = 1 */
    if (!exp_all_ones(hy)) {                       /* parity of y: r0 = y is an odd integer */
        double yi = dtrunc_base(y, 0x433);
        r0 = y == yi;
        double ye = dtrunc_base(yi, 0x434);
        if (r0) r0 = !(yi == ye);
    }
    if (inf_by_hi(hx)) {
        if (hx >> 31) {                            /* x = -Inf: sign follows y's parity */
            if (hy >> 31) return r0 ? K(0x8000000000000000) : 0.0;
            return r0 ? K(0xFFF0000000000000) : K(0x7FF0000000000000);
        }
        return (hy >> 31) ? 0.0 : K(0x7FF0000000000000);
    }
    if (inf_by_hi(hy)) {                           /* y = +-Inf, x finite */
        int e;
        frexp(x, &e);
        if (hy >> 31) return e <= 0 ? K(0x7FF0000000000000) : 0.0;
        return e > 0 ? K(0x7FF0000000000000) : 0.0;
    }
    if (r0) r0 = hx >> 31;                         /* x = +-0 */
    if (hy >> 31) return r0 ? K(0xFFF0000000000000) : K(0x7FF0000000000000);
    return r0 ? K(0x8000000000000000) : 0.0;
}

double pow(double x, double y)
{
    u64 yb = dbits(y);
    u32 hx = (u32)(dbits(x) >> 32), hy = (u32)(yb >> 32);
    int neg = 0, e;

    if (is_zero(y)) return 1.0;
    if (x == 1.0) return 1.0;
    if (x == -1.0 && inf_by_hi(hy)) return 1.0;
    if (exp_all_ones(hx) || is_zero(x) || exp_all_ones(hy))
        return special(x, y);

    double yi = dtrunc_base(y, 0x433);
    if ((hx >> 31) && y != yi) return K(0x7FFFFFFFFFFFFFFF);  /* negative x, non-integer y */

    double m = frexp(x, &e);
    if (dbits(m) >> 63) {
        m = bitsd(dbits(m) & 0x7FFFFFFFFFFFFFFFull);
        double ye = dtrunc_base(yi, 0x434);
        neg = !(yi == ye);
    }

    double c;
    int idx;
    if (e == 1 && m < K(0x3FE6A09E667F3BCD)) {
        idx = 0;
        c = 1.0;
        m = m * 2.0;
    } else {
        double kk = 0.5 + m * 64.0;
        s32 k = (s32)kk - 32;
        c = (double)k * K(0x3F90000000000000) + 0.5;   /* VCVT.F64.S32 #6 = k/64 */
        idx = k;
    }
    double f = m - c;
    double s = f / (m + c);
    double z = s * s;
    double p = K(0x3FC3991ACCFEB3F4) + z * K(0x3FC2F8D8BF29DEF7);
    p = K(0x3FC7466FD968CB62) + p * z;
    double omc = 1.0 - c;
    p = K(0x3FCC71C4DA12C9C6) + p * z;
    p = K(0x3FD24924948AE94A) + p * z;
    p = K(0x3FD9999999975FF7) + p * z;
    p = K(0x3FE55555555555BC) + p * z;
    double u = p * z;
    u = u - f;
    u = u + omc * 2.0;
    u = u * s;
    double lg = f + u;                              /* ln(m/c) */
    double t0 = (double)e + bitsd(T[idx][0]);
    double l2 = t0 + lg * K(0x3FF71547652B82FE);
    double t = l2 * y;
    double r;

    if (t < K(0xC0E0000000000000)) {
        r = 0.0;
    } else if (t >= K(0x40E0000000000001)) {
        r = K(0x7FF0000000000000);
    } else {
        s32 n = (dbits(t) >> 63) ? (s32)(t + -0.5) : (s32)(t + 0.5);
        double lo = bitsd(T[idx][1]) + bitsd(T[idx][2]);
        double a = lg * y;
        double b = t0 * y;
        lo = lo * y;
        b = b - (double)n;
        a = a + lo * K(0x3FE62E42FEFA39EF);
        a = a + b * K(0x3FE62E42FEFA39EF);
        r = __pow_core(a, 1.0, n);
        u32 rh = (u32)(dbits(r) >> 32);
        if (is_zero(r)) r = 0.0;
        else if (inf_by_hi(rh)) r = K(0x7FF0000000000000);
    }
    return neg ? -r : r;
}
