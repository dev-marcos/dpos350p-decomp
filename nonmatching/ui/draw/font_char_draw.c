/*
 * font_char_draw -- stock 0x4002E6D8 (216 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws glyph ch of the current font (word 0x42E68C74) at (x, y): rows 1 .. height-2 (u16 +4), each row starting at stride (u16 +2) * row + the glyph offset (+0xA + 2ch) in the glyph data, glyph width at +0x262 + 2ch; non-zero bytes become pixels.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

#define W32(a) (*(volatile u32 *)(a))
#define H16(a) (*(volatile unsigned short *)(a))

#define FONT (*(volatile u32 *)0x42E68C74u)

void font_char_draw(u32 x, u32 y, u32 ch)
{
    const unsigned char *bits = *(const unsigned char * volatile *)(FONT + 0x4BC);
    unsigned short r, k;
    ch = (unsigned short)ch;
    for (r = 0; r < H16(FONT + 4) - 1; r++) {
        u32 o = H16(FONT + 2) * (r + 1) + H16(FONT + 2 * ch + 0xA);
        u32 xx = (unsigned short)x, yy = r + (unsigned short)y;
        for (k = 0; k < H16(FONT + 2 * ch + 0x262); k++, o++, xx++)
            if (bits[(unsigned short)o])
                pixel_put((unsigned short)xx, (unsigned short)yy);
    }
}
