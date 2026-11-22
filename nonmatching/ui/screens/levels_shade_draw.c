/*
 * levels_shade_draw -- stock 0x40062B1C (352 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: for each level y from the min +0x168 (at least 0) below top +0xC0 of block 0x42E68668: colour from the 271-entry table 0x42E56C30 at (top - y) * 270 / (top - min), dimmed to (top - y) * 238 / (top - mean +0x160) + 0x11 of 255 above the mean (full below it); then a pixel at (x + +0xB0, y + +0xB4) for each x < +0xB8 whose level at 0x42E5F010 is at most y.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"

#define MI(o) (*(volatile int *)(0x42E68668u + (o)))
#define MD(o) (*(volatile double *)(0x42E68668u + (o)))
#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))

void levels_shade_draw(void)
{
    int top = MI(0xC0), y, x, d;
    double k1, k2;
    volatile short *lv;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    k1 = 238.0 / (double)(top - MI(0x160));
    k2 = 270.0 / (double)(top - MI(0x168));
    y = MI(0x168);
    if (y >= top)
        return;
    do {
        u32 col, a, rr, gg, bb;
        if (y < 0)
            y = 0;
        d = top - y;
        col = *(volatile u32 *)(0x42E56C30u + 4u * (u32)(int)((double)(d < 0 ? 0 : d) * k2));
        if (MI(0x160) < y)
            a = (u32)((double)d * k1) + 0x11;
        else
            a = 0xFF;
        rr = a * ((col >> 16) & 0xFF) / 255u;
        gg = a * ((col >> 8) & 0xFF) / 255u;
        bb = a * (col & 0xFF) / 255u;
        draw_colour_set((rr << 16) | (gg << 8) | bb);
        lv = (volatile short *)0x42E5F010u;
        for (x = 0; x < MI(0xB8); x++, lv++) {
            if (y >= *lv)
                pixel_put((unsigned short)(x + MI(0xB0)), (unsigned short)(y + MI(0xB4)));
        }
        y++;
    } while (y < MI(0xC0));
}
