/*
 * font_char_tint_draw -- stock 0x4002E878 (436 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Like font_char_draw, but each non-zero glyph byte v is an intensity: the drawing colour becomes v * (R, G, B) / 255 of the colour at 0x42E6865C (bytes +1..+3 and the RGB565 +4 rewritten) before pixel_put. The colour word +8 is restored at the end.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "shared/access.h"

#define CB8(o) (*(volatile unsigned char *)(0x42E6865Cu + (o)))
#define CB16(o) (*(volatile unsigned short *)(0x42E6865Cu + (o)))
#define FBP (*(volatile u32 *)IMG(40095498))

#define FONT (*(volatile u32 *)0x42E68C74u)

void font_char_tint_draw(u32 x, u32 y, u32 ch)
{
    int r0 = CB8(1), g0 = CB8(2), b0 = CB8(3);
    u32 saved = *(volatile u32 *)0x42E68664u;
    const unsigned char *bits = *(const unsigned char * volatile *)(FONT + 0x4BC);
    unsigned short r, k;
    ch = (unsigned short)ch;
    for (r = 0; r < H16(FONT + 4) - 1; r++) {
        u32 o = H16(FONT + 2) * (r + 1) + H16(FONT + 2 * ch + 0xA);
        u32 xx = (unsigned short)x, yy = r + (unsigned short)y;
        for (k = 0; k < H16(FONT + 2 * ch + 0x262); k++, o++, xx++) {
            int v = bits[(unsigned short)o], cr, cg, cb;
            if (v == 0)
                continue;
            cr = v * r0 / 255;
            cg = v * g0 / 255;
            cb = v * b0 / 255;
            CB8(3) = cb;
            CB8(1) = cr;
            CB8(2) = cg;
            CB16(4) = (((unsigned char)cg >> 2) << 5) | (((unsigned char)cr >> 3) << 11) | ((unsigned char)cb >> 3);
            pixel_put((unsigned short)xx, (unsigned short)yy);
        }
    }
    draw_colour_set(saved);
}
