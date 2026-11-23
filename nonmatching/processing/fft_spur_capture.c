/*
 * fft_spur_capture -- stock 0x40060BA4 (448 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Records the spurs of the spectrum 0x42D364B4 (float) for the mode word 0x42E68668+0xF4 (0..3; the stock leaves its pointers unset for other values): with half = (s32 +0xF8) / 2, mean = sum of bins 2 .. half - 1 / (half - 2) is stored, then every bin i < half with (s64)(v x 10000) > (s64)(mean x 3 x 10000) appends i to the u16 index list and v x 2 to the value list, and the u16 count is stored. Lists per mode at 0x42E3C000: values +0x2340 / +0x1D00 / +0x12D8 / +0x18, indices +0x37F4 / +0x34D4 / +0x2FC0 / +0x2660, mean +0x14 / +0x10 / +0xC / +8, count +6 / +4 / +2 / +0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "state/levels_block.h"

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

void fft_spur_capture(void)
{
    u32 vals, idxs, mean, cntp, i, cnt;
    int half;
    double sum = 0.0;
    float v;
    long long a;
    spur_lists(LEVELS(spur_mode), &vals, &idxs, &mean, &cntp);
    half = (int)LEVELS(fft_points) / 2;
    if ((u32)half >= 3)
        for (i = 2; i < (u32)half; i++)
            sum = (double)F32(SPEC + 4 * i) + sum;
    F32(mean) = (float)(sum / (double)(half - 2));
    cnt = 0;
    for (i = 0; i < (u32)half; i++) {
        v = F32(SPEC + 4 * i);
        a = F2LZ(v * 10000.0f);
        if (F2LZ(F32(mean) * 3.0f * 10000.0f) < a) {
            H16(cntp) = (unsigned short)cnt;
            H16(idxs + 2 * (cnt & 0xFFFF)) = (unsigned short)i;
            cnt = H16(cntp);
            F32(vals + 4 * cnt) = v * 2.0f;
            cnt++;
        }
    }
    H16(cntp) = (unsigned short)cnt;
}
