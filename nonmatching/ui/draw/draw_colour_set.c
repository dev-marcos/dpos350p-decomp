/*
 * draw_colour_set -- stock 0x4002EA38 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Colour c = 0xAARRGGBB into the block 0x42E6865C: +0 = A (pixel_put blends when it is non-zero), +1 = R, +2 = G, +3 = B, +8 = c, and +4 = the RGB565 value.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c)
{
    volatile unsigned char *p = (volatile unsigned char *)0x42E6865Cu;
    p[1] = c >> 16;
    p[2] = c >> 8;
    p[0] = c >> 24;
    p[3] = c;
    *(volatile u32 *)(p + 8) = c;
    *(volatile unsigned short *)(p + 4) = (((c >> 10) & 0x3F) << 5) | ((c >> 19) << 11) | ((c >> 3) & 0x1F);
}
