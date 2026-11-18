/*
 * text_width -- stock 0x4002D164 (368 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Width in pixels of the UTF-8 string s in the current font (word 0x42E68C74): for each character, the glyph width (u16 font + 0x262 + 2 * glyph) plus the spacing (u16 0x42E68BE2); minus one spacing at the end. Font type 1 (byte font + 0) uses font_glyph_index, and an unknown character counts 14 pixels (the 14 x 14 fallback glyph); type 8 uses char_to_digit_glyph and skips unknown characters; other types count nothing. The stock code keeps the length of an invalid lead byte from the previous character, and for the first character from the caller's r9 (the C starts at 1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

unsigned short font_glyph_index(u32 c);
unsigned short char_to_digit_glyph(u32 c);
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define FONT W32(0x42E68C74u)
#define SPACING H16(0x42E68BE2u)

/* One UTF-8 sequence at s: returns the code (only its low 16 bits are used) and sets *n to
   its length. An invalid lead byte gives code 0 and leaves *n unchanged. */
static u32 utf8_next(const char *s, unsigned char *n)
{
    int c = (signed char)s[0];
    u32 b = (unsigned char)c;
    if ((b >> 7) == 0) {
        *n = 1;
        return (u32)c;
    }
    if ((b >> 5) == 6) {
        *n = 2;
        return (((u32)c << 6) & 0x7C0) | ((unsigned char)s[1] & 0x3F);
    }
    if ((b >> 4) == 0xE) {
        *n = 3;
        return ((((u32)c << 6) | ((unsigned char)s[1] & 0x3F)) << 6) | ((unsigned char)s[2] & 0x3F);
    }
    if ((b >> 3) == 0x1E) {
        *n = 4;
        return ((((u32)(int)(signed char)s[1] << 6) | ((unsigned char)s[2] & 0x3F)) << 6) |
               ((unsigned char)s[3] & 0x3F);
    }
    return 0;
}

u32 text_width(const char *s)
{
    u32 sp = SPACING, font = FONT, w = 0;
    unsigned short i = 0;
    unsigned char n = 1;
    while ((unsigned char)s[i]) {
        u32 code = utf8_next(s + i, &n);
        u32 t = *(volatile unsigned char *)font;
        if (t == 1) {
            unsigned short g = font_glyph_index((unsigned short)code);
            if (g != 0xFFFF)
                w += sp + H16(font + 0x262 + 2u * g);
            else
                w += sp + 14;
        } else if (t == 8) {
            unsigned short g = char_to_digit_glyph((unsigned short)code);
            if (g != 0xFFFF)
                w += sp + H16(font + 0x262 + 2u * g);
        }
        i = (unsigned short)(i + n);
    }
    return w - sp;
}
