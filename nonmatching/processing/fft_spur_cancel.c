/*
 * fft_spur_cancel -- stock 0x40060DC8 (544 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set: Hann window of the n = 0x42E68668+0xF8 floats at 0x42D164B4 into complex 0x42BEFE74 (as fft_hann_spectrum), byte +4 = 1, fft_radix2(n), cvec_magnitude into 0x42D364B4 (n / 2 bins). Then, unless byte +0x190 is set, each spur recorded by fft_spur_capture for the mode +0xF4 is subtracted from its bin, and a bin that falls below the stored mean ((s64) x 10000 compare) is set to the mean.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "state/levels_block.h"

#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define F32(a) (*(volatile float *)(a))

#define BLK 0x42E68668u
#define SPT 0x42E3C000u
#define SPEC 0x42D364B4u
typedef long long (*f2lz_fn)(float) __attribute__((pcs("aapcs")));
#define F2LZ(x) (((f2lz_fn)0x4006A4E9u)(x))   /* __aeabi_f2lz at 0x4006A4E8 is Thumb */

/* per mode word BLK+0xF4 = 0..3: values (float), bin indices (u16), mean (float), count (u16) */
static void spur_lists(u32 sel, u32 *vals, u32 *idxs, u32 *mean, u32 *cntp)
{
    switch (sel) {
    case 0: *vals = SPT + 0x2340; *idxs = SPT + 0x37F4; *mean = SPT + 0x14; *cntp = SPT + 6; break;
    case 1: *vals = SPT + 0x1D00; *idxs = SPT + 0x34D4; *mean = SPT + 0x10; *cntp = SPT + 4; break;
    case 2: *vals = SPT + 0x12D8; *idxs = SPT + 0x2FC0; *mean = SPT + 0xC; *cntp = SPT + 2; break;
    default: *vals = SPT + 0x18; *idxs = SPT + 0x2660; *mean = SPT + 8; *cntp = SPT; break;
    }
}

#define HIN 0x42D164B4u
#define HX 0x42BEFE74u
typedef double (*dfn)(double);
#define COS_T ((dfn)0x40066531u)   /* cos at 0x40066530 is Thumb */

void fft_spur_cancel(void)
{
    u32 i, lim, vals, idxs, mean, cntp, k, cnt, idx;
    double dn, v;
    float s, m;
    long long a;
    if (B8(0x42E68BB0u + 0x17))
        return;
    i = 0;
    if (LEVELS(fft_points) != 0) {
        dn = (double)(int)(LEVELS(fft_points) - 1);
        do {
            v = (double)F32(HIN + 4 * i);
            lim = LEVELS(fft_points);
            F32(HX + 8 * i) = (float)(v * (0.5 - COS_T((double)i * 6.283185307179586 / dn) * 0.5));
            F32(HX + 8 * i + 4) = 0.0f;
            i++;
        } while (i < lim);
    }
    LEVELS(f_004) = 1;
    fft_radix2((int)LEVELS(fft_points), (volatile float *)HX);
    cvec_magnitude((volatile float *)HX, (volatile float *)SPEC, (int)LEVELS(fft_points) / 2);
    if (LEVELS(f_190))
        return;
    spur_lists(LEVELS(spur_mode), &vals, &idxs, &mean, &cntp);
    cnt = H16(cntp);
    for (k = 0; k < cnt; k++) {
        idx = H16(idxs + 2 * k);
        s = F32(SPEC + 4 * idx) - F32(vals + 4 * k);
        F32(SPEC + 4 * idx) = s;
        m = F32(mean);
        a = F2LZ(m * 10000.0f);
        if (F2LZ(s * 10000.0f) < a)
            F32(SPEC + 4 * idx) = m;
    }
}
