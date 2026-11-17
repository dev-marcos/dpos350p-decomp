/*
 * vline_draw -- stock 0x4002EBC8 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Vertical line at x from y0 to y1 inclusive. With a non-zero alpha (byte 0x42E6865C) each pixel goes through pixel_put (blending); otherwise the RGB565 colour (0x42E68660) is stored directly, stride 1022.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define CB8(o) (*(volatile unsigned char *)(0x42E6865Cu + (o)))
#define CB16(o) (*(volatile unsigned short *)(0x42E6865Cu + (o)))
#define FBP (*(volatile u32 *)0x40095498u)
void pixel_put(u32 x, u32 y);

void vline_draw(u32 x, u32 y0, u32 y1)
{
    int end = y1 + 1;
    u32 y = y0;
    if (CB8(0)) {
        if ((int)y0 >= end)
            return;
        do {
            pixel_put(x, (unsigned short)y);
            y++;
        } while (end > (int)(unsigned short)y);
    } else {
        volatile unsigned short *p;
        int n;
        if ((int)y0 >= end)
            return;
        p = (volatile unsigned short *)(FBP + (x + y0 * 1022u) * 2u);
        n = end - y0;
        do {
            *p = CB16(4);
            p += 1022;
        } while (--n);
    }
}
