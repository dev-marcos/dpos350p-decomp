/*
 * rect_fill -- stock 0x4002ECC4 (204 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fills w x h pixels at (x, y). With a non-zero alpha each pixel is blended (pixel_blend); otherwise each row is filled with the RGB565 colour (fill_u16), the drawing target pointer read again for every row.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define CB8(o) (*(volatile unsigned char *)(0x42E6865Cu + (o)))
#define CB16(o) (*(volatile unsigned short *)(0x42E6865Cu + (o)))
#define FBP (*(volatile u32 *)0x40095498u)
void pixel_put(u32 x, u32 y);

void pixel_blend(u32 x, u32 y);
void fill_u16(unsigned short *d, unsigned short v, u32 n);

void rect_fill(u32 x, u32 y, u32 w, u32 h)
{
    if (CB8(0)) {
        int ye = y + h, xe = x + w;
        u32 yy = y;
        if ((int)y >= ye)
            return;
        do {
            if ((int)x < xe) {
                u32 xx = x;
                do {
                    pixel_blend((unsigned short)xx, (unsigned short)yy);
                    xx++;
                } while (xe > (int)(unsigned short)xx);
            }
            yy++;
        } while (ye > (int)(unsigned short)yy);
    } else {
        int ye = (unsigned short)(h + y), n;
        u32 o;
        if ((int)y >= ye)
            return;
        n = ye - y;
        o = y * 511u * 4u;
        do {
            fill_u16((unsigned short *)(FBP + x * 2u + o), CB16(4), w);
            o += 0x7FC;
        } while (--n);
    }
}
