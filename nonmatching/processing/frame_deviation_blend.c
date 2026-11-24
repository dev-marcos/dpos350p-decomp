/*
 * frame_deviation_blend -- stock 0x40041118 (524 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Blend of the F = SYS16+0x550 frames in the display buffer of a channel (frame k at index SYS+0x2C4[k], L = SYS+0x2B0 points): per point the mean over frames (to 0x42C947D4), per frame the largest deviation from the mean (starting at (1 - f) x lim, capped at lim = SYS+0x1C0 x n), then every sample = (w x sample + (100 - w) x mean) / 100 with w = 100 x deviation / lim. The stock code leaves the divisions by zero to the hardware (result 0); the C does the same explicitly.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "util.h"
#include "sys_info.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void fill_u32_raw(volatile u32 *d, u32 v, u32 n) __asm__("fill_u32");

void frame_deviation_blend(u32 ch, u32 n, float fr)
{
    volatile unsigned short *buf = (volatile unsigned short *)(0x42AC7CF4u + ch * 0x62250u);
    volatile u32 *mean = (volatile u32 *)0x42C947D4u;
    volatile u32 *off = (volatile u32 *)(0x40544E68u + 0x2C4);
    int dev[100];
    int lim = (int)(SI(acq_geometry_1c0) * n), init;
    u32 L = SI(block_points), F, j, k;
    init = (int)((1.0 - (double)fr) * (double)(float)lim);
    if (L != 0) {
        F = SI(capture_count);
        for (j = 0; j < L; j++) {
            u32 s = 0;
            for (k = 0; k < SI(capture_count); k++)
                s += buf[j + off[k]];
            mean[j] = F ? s / F : 0;
        }
    }
    fill_u32_raw((volatile u32 *)dev, (u32)init, 100);
    F = SI(capture_count);
    L = SI(block_points);
    for (j = 0; j < L; j++) {
        u32 m;
        if (F == 0)
            continue;
        m = mean[j];
        for (k = 0; k < SI(capture_count); k++) {
            int d = (int)(buf[j + off[k]] - m);
            if (d < 0) d = -d;
            if (d < dev[k]) d = dev[k];
            dev[k] = d;
        }
    }
    for (k = 0; k < SI(capture_count); k++)
        if (lim <= dev[k])
            dev[k] = lim;
    for (j = 0; j < SI(block_points); j++) {
        u32 m = mean[j];
        for (k = 0; k < SI(capture_count); k++) {
            int w = lim ? (int)((u32)dev[k] * 100u) / lim : 0;
            u32 p = j + off[k];
            buf[p] = ((u32)(100 - w) * m + (u32)w * buf[p]) / 100;
        }
    }
}
