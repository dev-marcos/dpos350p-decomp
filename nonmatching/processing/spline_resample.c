/*
 * spline_resample -- stock 0x4005CF78 (208 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Cubic spline through the n points (xk, yk): coefficients from 0x4005C95C (natural ends: flags 0, end slopes 0.0) into b / c / d = 0x42D6EB54 / 0x42D823D4 / 0x42D95C54, then yout[j] = spline_eval(xin[j]) for j < m, keeping the segment index between calls.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define F32(a) (*(volatile float *)(a))

#define SPL_B 0x42D6EB54u
#define SPL_C 0x42D823D4u
#define SPL_D 0x42D95C54u

void spline_resample(int n, const volatile float *xk, const volatile float *yk, u32 m, const volatile float *xin,
                     volatile float *yout)
{
    volatile u32 work = 0;
    volatile int idx = 0;
    u32 k;
    spline_coeffs(n, 0, 0, xk, yk, (volatile float *)SPL_B, (volatile float *)SPL_C, (volatile float *)SPL_D, &work,
                  0.0f, 0.0f);
    for (k = m; k; k--)
        *yout++ = spline_eval(m, n, xk, yk, (volatile float *)SPL_B, (volatile float *)SPL_C,
                              (volatile float *)SPL_D, &idx, *xin++);
}
