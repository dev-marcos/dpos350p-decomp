/*
 * floating_point_interpolation_resampling -- stock 0x4005D048 (472 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Spline zoom of the u16 workspace ws around index half: with k = SYS+0x554 (current zoom) and m the target, c = (len x k / m) x 12 / 10 input samples starting c/2 before half are turned into floats (v - 128 x SYS+0x1C0) / (128 x SYS+0x1C0) at 0x42DBCD54; cnt = m x c / k outputs at x = j x k / m (knots x = j at 0x42DA94D4, positions at 0x42DD05D4) go through spline_resample into 0x42DE3E54 and back to u16 (v + 1) x scale, negative values to 0, at 0x416431E8; then len samples, centred and moved back by the 64-bit stack argument, are copied to ws from half - len/2.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "util.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define F32(a) (*(volatile float *)(a))

#define FY 0x42DBCD54u
#define FX 0x42DA94D4u
#define FQ 0x42DD05D4u
#define FO 0x42DE3E54u
#define OUT16 0x416431E8u

void floating_point_interpolation_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift)
{
    u32 k = SI(f_554), lv = SI(acq_geometry_1c0), sb = lv << 7, cnt, src, dst, i;
    float ratio = (float)m / (float)k, scale, inv;
    long long t;
    int c, j, v;
    double sc = (double)lv * 128.0;
    t = (long long)((unsigned long long)len * k) / (long long)m;
    c = (int)((long long)((unsigned long long)t * 12) / 10);
    scale = (float)sc;
    src = ws + half * 2 - ((u32)(c + (int)((u32)c >> 31)) & ~1u);
    dst = ws + half * 2 - (len & ~1u);
    cnt = (u32)c * m / k;
    if (c >= 0) {
        j = 0;
        do {
            v = (int)(H16(src) - sb);
            src += 2;
            F32(FY + 4 * j) = (float)v / scale;
            j++;
        } while (c >= j);
    }
    if ((int)cnt > 0) {
        inv = 1.0f / ratio;
        for (j = 0; j < (int)cnt; j++) {
            F32(FX + 4 * j) = (float)j;
            F32(FQ + 4 * j) = (float)j * inv;
        }
    }
    spline_resample(c, (const volatile float *)FX, (const volatile float *)FY, cnt, (const volatile float *)FQ,
                    (volatile float *)FO);
    if ((int)cnt > 0)
        for (i = 0; i < cnt; i++) {
            v = (int)((F32(FO + 4 * i) + 1.0f) * scale);
            if (v < 0)
                v = 0;
            H16(OUT16 + 2 * i) = (unsigned short)v;
        }
    copy_u16((unsigned short *)dst,
             (const unsigned short *)(OUT16 + ((cnt - len + ((cnt - len) >> 31)) & ~1u) - ((u32)shift << 1)), len);
}
