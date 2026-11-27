/*
 * pixel_blend -- stock 0x4002EA88 (232 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Blends the drawing colour into the pixel (x, y) of the drawing target: each RGB565 channel is widened to 8 bits (v * 8 + 7, or 0), mixed as (A * pixel + (255 - A) * colour) / 255 with A, R, G, B from 0x42E6865C, and packed back.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void pixel_blend(u32 x, u32 y)
{
    volatile unsigned short *px = (volatile unsigned short *)(*(volatile u32 *)IMG(40095498) + (x + y * 1022u) * 2u);
    volatile unsigned char *c = (volatile unsigned char *)0x42E6865Cu;
    int a = c[0], cr = c[1], cg = c[2], cb = c[3], v = *px, r, g, b;
    r = (v >> 11) & 31;
    g = (v >> 5) & 63;
    b = v & 31;
    r = (unsigned char)(r ? r * 8 + 7 : 0);
    g = (unsigned char)(g ? g * 4 + 3 : 0);
    b = (unsigned char)(b ? b * 8 + 7 : 0);
    r = (a * r + (255 - a) * cr) / 255;
    g = (a * g + (255 - a) * cg) / 255;
    b = (a * b + (255 - a) * cb) / 255;
    *px = (((unsigned char)r >> 3) << 11) | (((unsigned char)g >> 2) << 5) | ((unsigned char)b >> 3);
}
