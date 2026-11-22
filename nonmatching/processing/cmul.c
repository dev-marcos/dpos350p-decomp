/*
 * cmul -- stock 0x40066584 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Complex float product (a.re b.re - a.im b.im, a.re b.im + a.im b.re), returned in s0/s1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

typedef struct { float re, im; } cfloat;

cfloat cmul(cfloat a, cfloat b)
{
    cfloat r;
    r.re = a.re * b.re - a.im * b.im;
    r.im = a.re * b.im + a.im * b.re;
    return r;
}
