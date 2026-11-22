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

#define MI(o) (*(volatile int *)(0x42E68668u + (o)))
#define MD(o) (*(volatile double *)(0x42E68668u + (o)))
#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))

void marker_values_compute(void)
{
    double span = (double)M64(0x138), n = (double)MI(0xB8), lo = (double)M64(0x120);
    double sc, top, stp, y0, y1, l0, l1, p0, p1;
    long long a, b;
    unsigned long long d;
    a = (long long)((double)MI(0x5C) / n * span + lo);
    b = (long long)((double)MI(0x60) / n * span + lo);
    d = (unsigned long long)b - (unsigned long long)a;
    if ((long long)d < 0)
        d = 0 - d;                   /* llabs */
    M64(0x70) = b;
    M64(0x68) = a;
    M64(0x78) = (long long)d;
    sc = (double)MI(0xCC);
    top = MD(0x148);
    stp = MD(0x140);
    y1 = (double)*(volatile short *)(0x42E5F010u + 2u * (u32)MI(0x60));
    y0 = (double)*(volatile short *)(0x42E5F010u + 2u * (u32)MI(0x5C));
    l0 = top - y0 / sc * stp;
    l1 = top - y1 / sc * stp;
    MD(0x88) = l1;
    MD(0x90) = __builtin_fabs(l1 - l0);
    MD(0x80) = l0;
    p0 = pow(10.0, MD(0x80) / 20.0) * 1000.0;
    p1 = pow(10.0, MD(0x88) / 20.0) * 1000.0;
    MD(0x98) = p0;
    MD(0xA8) = __builtin_fabs(p1 - p0);
    MD(0xA0) = p1;
}
