/*
 * marker_values_compute -- stock 0x4006053C (328 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Block 0x42E68668: the two marker positions (words +0x5C, +0x60 out of n = +0xB8 points) as s64 values lo + pos / n * span (lo = +0x120, span = +0x138) into +0x68 / +0x70 and their distance into +0x78; their levels top - y / scale * step (top +0x148, step +0x140, scale +0xCC, y from the u16 levels at 0x42E5F010) into +0x80 / +0x88 and the distance into +0x90; then 1000 * 10^(level / 20) into +0x98 / +0xA0 and the distance into +0xA8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "libm.h"
#include "processing.h"
#include "state/levels_block.h"

#define MI(o) (*(volatile int *)(0x42E68668u + (o)))

void marker_values_compute(void)
{
    double span = (double)LEVELS(freq_span), n = (double)MI(0xB8), lo = (double)LEVELS(start_freq);
    double sc, top, stp, y0, y1, l0, l1, p0, p1;
    long long a, b;
    unsigned long long d;
    a = (long long)((double)LEVELS(marker1_x) / n * span + lo);
    b = (long long)((double)LEVELS(marker2_x) / n * span + lo);
    d = (unsigned long long)b - (unsigned long long)a;
    if ((long long)d < 0)
        d = 0 - d;                   /* llabs */
    LEVELS(c2_freq) = b;
    LEVELS(c1_freq) = a;
    LEVELS(dc_freq) = (long long)d;
    sc = (double)MI(0xCC);
    top = LEVELS(reference);
    stp = LEVELS(attenuation);
    y1 = (double)*(volatile short *)(0x42E5F010u + 2u * (u32)LEVELS(marker2_x));
    y0 = (double)*(volatile short *)(0x42E5F010u + 2u * (u32)LEVELS(marker1_x));
    l0 = top - y0 / sc * stp;
    l1 = top - y1 / sc * stp;
    LEVELS(c2_level) = l1;
    LEVELS(dc_level) = __builtin_fabs(l1 - l0);
    LEVELS(c1_level) = l0;
    p0 = pow(10.0, LEVELS(c1_level) / 20.0) * 1000.0;
    p1 = pow(10.0, LEVELS(c2_level) / 20.0) * 1000.0;
    LEVELS(c1_volts) = p0;
    LEVELS(dc_volts) = __builtin_fabs(p1 - p0);
    LEVELS(c2_volts) = p1;
}
