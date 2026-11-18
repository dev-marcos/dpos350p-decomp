/*
 * spline_eval -- stock 0x4005CEA4 (212 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Evaluates a cubic spline at x (s0): segment i = *idx if 0 <= i < n - 1 and xk[i] <= x <= xk[i + 1], otherwise found by bisection over xk[0..n]; *idx = i and the result is a[i] + dx (b[i] + dx (c[i] + dx d[i])) with dx = x - xk[i]. The first argument is not used (callers pass the output count).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define F32(a) (*(volatile float *)(a))

float spline_eval(u32 unused, int n, const volatile float *xk, const volatile float *a, const volatile float *b,
                  const volatile float *c, const volatile float *d, volatile int *idx, float x)
{
    int i = *idx, lo, hi, mid;
    float v, dx;
    (void)unused;
    if ((u32)i >= (u32)(n - 1) || i < 0)
        i = 0;
    if (x < xk[i] || xk[i + 1] < x) {
        lo = 0;
        hi = n;
        do {
            mid = (hi + lo) / 2;
            v = xk[mid];
            if (x < v)
                hi = mid;
            if (x >= v)
                lo = mid;
        } while (lo + 1 < hi);
        i = lo;
    }
    *idx = i;
    dx = x - xk[i];
    return a[i] + (b[i] + (c[i] + dx * d[i]) * dx) * dx;
}
