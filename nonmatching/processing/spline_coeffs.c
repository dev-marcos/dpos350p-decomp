/*
 * spline_coeffs -- stock 0x4005C95C (1352 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Cubic spline coefficients through the n points (x, y): *iflag = 1 and nothing else when n <= 1 (unsigned), *iflag = 2 when the x are not strictly increasing; n = 2 gives the straight line (c = d = 0). Otherwise the tridiagonal system is solved with h = x[i+1] - x[i] kept in d, the diagonal in b and the right-hand side in c; the end conditions are third-derivative matching (n > 3, zero for n = 3), replaced by the slope slope1 (end1 = 1) / slope2 (end2 = 1). On return y[i] + b dx + c dx^2 + d dx^3 is the spline on [x[i], x[i+1]] (the form used by spline_eval). Float arithmetic in the stock order; the return value is not set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define F32(a) (*(volatile float *)(a))
#define D64(a) (*(volatile double *)(a))

void spline_coeffs(int n, int end1, int end2, const volatile float *x, const volatile float *y, volatile float *b,
                   volatile float *c, volatile float *d, volatile u32 *iflag, float slope1, float slope2)
{
    u32 un = (u32)n;
    int nm1 = n - 1, i, ok, ib;
    float t;
    *iflag = 0;
    if (un <= 1) {
        *iflag = 1;
        return;
    }
    ok = 1;
    for (i = 0; i < nm1; i++)
        if (x[i] >= x[i + 1])
            ok = 0;
    if (!ok) {
        *iflag = 2;
        return;
    }
    if (un <= 2) {
        float h = x[1] - x[0];
        b[0] = (y[1] - y[0]) / h;
        c[0] = 0.0f;
        d[0] = 0.0f;
        b[1] = b[0];
        c[1] = 0.0f;
        d[1] = 0.0f;
        return;
    }
    d[0] = x[1] - x[0];
    c[1] = (y[1] - y[0]) / d[0];
    for (i = 1; i < nm1; i++) {
        d[i] = x[i + 1] - x[i];
        b[i] = (d[i - 1] + d[i]) * 2.0f;
        c[i + 1] = (y[i + 1] - y[i]) / d[i];
        c[i] = c[i + 1] - c[i];
    }
    b[0] = -d[0];
    b[nm1] = -d[n - 2];
    c[0] = 0.0f;
    c[nm1] = 0.0f;
    if (n != 3) {
        c[0] = c[2] / (x[3] - x[1]) - c[1] / (x[2] - x[0]);
        c[nm1] = c[n - 2] / (x[nm1] - x[n - 3]) - c[n - 3] / (x[n - 2] - x[n - 4]);
        c[0] = c[0] * d[0] * d[0] / (x[3] - x[0]);
        c[nm1] = -(c[nm1] * d[n - 2] * d[n - 2] / (x[nm1] - x[n - 4]));
    }
    if (end1 == 1) {
        b[0] = (x[1] - x[0]) * 2.0f;
        c[0] = (y[1] - y[0]) / (x[1] - x[0]) - slope1;
    }
    if (end2 == 1) {
        b[nm1] = (x[nm1] - x[n - 2]) * 2.0f;
        c[nm1] = slope2 - (y[nm1] - y[n - 2]) / (x[nm1] - x[n - 2]);
    }
    for (i = 1; (u32)i < un; i++) {
        t = d[i - 1] / b[i - 1];
        b[i] = b[i] - t * d[i - 1];
        c[i] = c[i] - t * c[i - 1];
    }
    c[nm1] = c[nm1] / b[nm1];
    for (ib = 0; ib < nm1; ib++) {
        i = n - ib - 2;
        c[i] = (c[i] - d[i] * c[i + 1]) / b[i];
    }
    b[nm1] = (y[nm1] - y[n - 2]) / d[n - 2] + (c[n - 2] + c[nm1] * 2.0f) * d[n - 2];
    for (i = 0; i < nm1; i++) {
        b[i] = (y[i + 1] - y[i]) / d[i] - (c[i + 1] + c[i] * 2.0f) * d[i];
        d[i] = (c[i + 1] - c[i]) / d[i];
        c[i] = c[i] * 3.0f;
    }
    c[nm1] = c[nm1] * 3.0f;
    d[nm1] = d[n - 2];
}
