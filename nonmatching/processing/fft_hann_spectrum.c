/*
 * fft_hann_spectrum -- stock 0x40041BA0 (408 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For each channel that is on (CH1: bytes SYS+0x3E and +0x42; CH2: +0x102 and +0x106) with n = SYS+0x7F8: x[i] = in[i] x (0.5 - 0.5 cos(2 pi i / (n - 1))) as complex floats with a zero imaginary part (CH1: in 0x42E49EB4, x 0x42E2DB08; CH2: in 0x42E4DEB4, x 0x42E31B08), then fft_radix2(n, x) and cvec_magnitude(x, out, n / 2) with out 0x42E4BEB4 / 0x42E4FEB4. The FFT is called even when n is 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

#define SYS 0x40544E68u

typedef struct { float re, im; } cfloat;
typedef double (*dfn)(double);
#define COS_T ((dfn)CODE_ADDR(cos))   /* cos at 0x40066530 is Thumb */
#define SIN_T ((dfn)CODE_ADDR(sin))   /* sin at 0x40066534 is Thumb */

static void hann_one(u32 in, u32 x, u32 out)
{
    volatile float *src = (volatile float *)in, *dst = (volatile float *)x;
    u32 n = SI(fft_size), i;
    double dn, v;
    if (n != 0) {
        dn = (double)(n - 1);
        i = 0;
        do {
            v = (double)src[i];
            dst[2 * i] = (float)(v * (0.5 - COS_T((double)i * 6.283185307179586 / dn) * 0.5));
            dst[2 * i + 1] = 0.0f;
            i++;
        } while (i < n);
    }
    fft_radix2((int)n, dst);
    cvec_magnitude(dst, (volatile float *)out, (int)(SI(fft_size) >> 1));
}

void fft_hann_spectrum(void)
{
    if (SI(ch1_enabled) && SI(ch1_fft_enabled))
        hann_one(0x42E49EB4u, 0x42E2DB08u, 0x42E4BEB4u);
    if (SI(ch2_enabled) && SI(ch2_fft_enabled))
        hann_one(0x42E4DEB4u, 0x42E31B08u, 0x42E4FEB4u);
}
