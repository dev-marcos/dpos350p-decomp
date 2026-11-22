/*
 * text_draw -- stock 0x4002CE7C (600 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the UTF-8 string s at (x, y) in the current font (word 0x42E68C74) and stores the drawn width + 1 in u16 0x42E68BE4. Font type 1: font_char_draw of font_glyph_index, or for unknown characters glyph_14x14_draw of the code from table u16 0x42CF64B4[code] (advance 14). Font type 8: font_char_tint_draw of char_to_digit_glyph (unknown characters skipped). The advance is the glyph width (font + 0x262 + 2 * glyph) plus the spacing u16 0x42E68BE2. With byte 0x42E68BE0 set, each glyph is centred on x (x - width / 2) and the advance is the fixed width u16 font + 8. Same UTF-8 decoding as text_width.
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

void glyph_14x14_draw(u32 x, u32 y, unsigned short code);
void font_char_draw(u32 x, u32 y, u32 ch);
void font_char_tint_draw(u32 x, u32 y, u32 ch);
#define CENTRED (*(volatile unsigned char *)0x42E68BE0u)

void text_draw(u32 x, u32 y, const char *s)
{
    u32 x0 = (unsigned short)x;
    unsigned short i = 0;
    unsigned char n = 1;
    y = (unsigned short)y;
    while ((unsigned char)s[i]) {
        u32 code = utf8_next(s + i, &n);
        u32 font = FONT, t = *(volatile unsigned char *)font;
        if (t == 1) {
            unsigned short g = font_glyph_index((unsigned short)code);
            if (g == 0xFFFF) {
                glyph_14x14_draw((unsigned short)x, y, H16(0x42CF64B4u + 2u * (unsigned short)code));
                x = SPACING + x + 14;
            } else if (CENTRED == 0) {
                font_char_draw((unsigned short)x, y, g);
                x = SPACING + (H16(FONT + 0x262 + 2u * g) + x);
            } else {
                font_char_draw((unsigned short)(x - (H16(font + 0x262 + 2u * g) >> 1)), y, g);
                x = SPACING + (H16(FONT + 8) + x);
            }
        } else if (t == 8) {
            unsigned short g = char_to_digit_glyph((unsigned short)code);
            if (g != 0xFFFF) {
                if (CENTRED == 0) {
                    font_char_tint_draw((unsigned short)x, y, g);
                    x = SPACING + (H16(FONT + 0x262 + 2u * g) + x);
                } else {
                    font_char_tint_draw((unsigned short)(x - (H16(font + 0x262 + 2u * g) >> 1)), y, g);
                    x = SPACING + (H16(FONT + 8) + x);
                }
            }
        }
        i = (unsigned short)(i + n);
    }
    H16(0x42E68BE4u) = (unsigned short)(x - x0 + 1);
}
