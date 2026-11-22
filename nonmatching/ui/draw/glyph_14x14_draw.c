/*
 * glyph_14x14_draw -- stock 0x4002CDC4 (184 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the 14 x 14 glyph of the two-byte code (hi * 94 + lo - 0x4141, 28 bytes each, 2 per row, MSB first) from the font at 0x40000040 with pixel_put at (x, y). The code layout matches GB2312 rows/columns (DERIVED).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

#define W32(a) (*(volatile u32 *)(a))
#define H16(a) (*(volatile unsigned short *)(a))

void glyph_14x14_draw(u32 x, u32 y, unsigned short code)
{
    const unsigned char *p = (const unsigned char *)0x40000040u +
        (unsigned short)((code >> 8) * 0x5E + (code & 0xFF) - 0x4141) * 28u;
    int row, col;
    for (row = 0; row < 14; row++)
        for (col = 0; col < 14; col++)
            if (p[2 * row + (col >> 3)] & (0x80 >> (col & 7)))
                pixel_put((unsigned short)(x + col), (unsigned short)(y + row));
}
