/*
 * cvec_magnitude -- stock 0x40066538 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * dst[i] = sqrtf-by-double(re^2 + im^2) for n complex floats (n <= 0: nothing).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

typedef double (*sqrt_fn)(double);
#define SQRT_T ((sqrt_fn)0x4003DF75u)   /* sqrt at 0x4003DF74 is Thumb */

void cvec_magnitude(const volatile float *src, volatile float *dst, int n)
{
    for (; n >= 1; n--) {
        float re = src[0], im = src[1];
        float m = re * re + im * im;
        src += 2;
        *dst++ = (float)SQRT_T((double)m);
    }
}
