# Text and number rendering: UTF-8 string width and drawing, the font glyph index, number
# formatting (width and drawing) and alpha-mask glyphs.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wm(addr, size, name, desc, body, src="display"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


TXT = """unsigned short font_glyph_index(u32 c);
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
"""

wm(0x4002DD70, 2408, "font_glyph_index",
   "Glyph index of character code c in the proportional fonts (font type 1): 'A'..'Z' = 0..25, 'a'..'z' = 26..51, Cyrillic 52..99, accented Latin and German letters 100..109, '0'..'9' = 110..119, punctuation 120..134. Accented vowels with pinyin tones map to the base letter. Any other code gives 0xFFFF. The full 32-bit value is compared. The table was read from the stock code (a 186-entry switch).", """
unsigned short font_glyph_index(u32 c)
{
    if (c - 0x30u <= 9)
        return (unsigned short)(c - 0x30u + 110);
    if (c - 0x41u <= 25)
        return (unsigned short)(c - 0x41u);
    if (c - 0x61u <= 25)
        return (unsigned short)(c - 0x61u + 26);
    switch (c) {
    case 0x0020:
        return 134;
    case 0x0021:
        return 126;
    case 0x0025:
        return 125;
    case 0x0028: case 0xFF08:
        return 127;
    case 0x0029: case 0xFF09:
        return 128;
    case 0x002A:
        return 123;
    case 0x002B:
        return 121;
    case 0x002C:
        return 132;
    case 0x002D:
        return 122;
    case 0x002E:
        return 120;
    case 0x002F:
        return 124;
    case 0x003A:
        return 131;
    case 0x003D:
        return 130;
    case 0x003F:
        return 133;
    case 0x005F:
        return 129;
    case 0x00C4:
        return 102;
    case 0x00D1:
        return 100;
    case 0x00D6:
        return 103;
    case 0x00DC:
        return 104;
    case 0x00DF:
        return 109;
    case 0x00E0: case 0x00E1: case 0x00E3: case 0x0101: case 0x01CE: case 0x0430:
        return 26;
    case 0x00E4:
        return 106;
    case 0x00E7: case 0x0441:
        return 28;
    case 0x00E8: case 0x00E9: case 0x00EA: case 0x0113: case 0x011B: case 0x0435:
        return 30;
    case 0x00EC: case 0x00ED: case 0x012B: case 0x01D0:
        return 34;
    case 0x00F1:
        return 101;
    case 0x00F2: case 0x00F3: case 0x00F5: case 0x014D: case 0x01D2: case 0x043E:
        return 40;
    case 0x00F6:
        return 107;
    case 0x00F9: case 0x00FA: case 0x016B: case 0x01D4:
        return 46;
    case 0x00FC: case 0x01D6: case 0x01D8: case 0x01DA: case 0x01DC:
        return 108;
    case 0x0144: case 0x0148:
        return 39;
    case 0x0261:
        return 32;
    case 0x0401:
        return 55;
    case 0x0410:
        return 0;
    case 0x0411:
        return 52;
    case 0x0412:
        return 1;
    case 0x0413:
        return 53;
    case 0x0414:
        return 54;
    case 0x0415:
        return 4;
    case 0x0416:
        return 56;
    case 0x0417:
        return 57;
    case 0x0418:
        return 58;
    case 0x0419:
        return 59;
    case 0x041A:
        return 10;
    case 0x041B:
        return 60;
    case 0x041C:
        return 12;
    case 0x041D:
        return 7;
    case 0x041E:
        return 14;
    case 0x041F:
        return 61;
    case 0x0420:
        return 15;
    case 0x0421:
        return 2;
    case 0x0422:
        return 19;
    case 0x0423:
        return 62;
    case 0x0424:
        return 63;
    case 0x0425:
        return 23;
    case 0x0426:
        return 64;
    case 0x0427:
        return 65;
    case 0x0428:
        return 66;
    case 0x0429:
        return 67;
    case 0x042A:
        return 68;
    case 0x042B:
        return 69;
    case 0x042C:
        return 70;
    case 0x042D:
        return 71;
    case 0x042E:
        return 72;
    case 0x042F:
        return 73;
    case 0x0431:
        return 74;
    case 0x0432:
        return 75;
    case 0x0433:
        return 76;
    case 0x0434:
        return 77;
    case 0x0436:
        return 79;
    case 0x0437:
        return 80;
    case 0x0438:
        return 81;
    case 0x0439:
        return 82;
    case 0x043A:
        return 83;
    case 0x043B:
        return 84;
    case 0x043C:
        return 85;
    case 0x043D:
        return 86;
    case 0x043F:
        return 87;
    case 0x0440:
        return 41;
    case 0x0442:
        return 88;
    case 0x0443:
        return 50;
    case 0x0444:
        return 89;
    case 0x0445:
        return 49;
    case 0x0446:
        return 90;
    case 0x0447:
        return 91;
    case 0x0448:
        return 92;
    case 0x0449:
        return 93;
    case 0x044A:
        return 94;
    case 0x044B:
        return 95;
    case 0x044C:
        return 96;
    case 0x044D:
        return 97;
    case 0x044E:
        return 98;
    case 0x044F:
        return 99;
    case 0x0451:
        return 78;
    case 0x1E9E:
        return 105;
    default:
        return 0xFFFF;
    }
}
""")

wm(0x4002D164, 368, "text_width",
   "Width in pixels of the UTF-8 string s in the current font (word 0x42E68C74): for each character, the glyph width (u16 font + 0x262 + 2 * glyph) plus the spacing (u16 0x42E68BE2); minus one spacing at the end. Font type 1 (byte font + 0) uses font_glyph_index, and an unknown character counts 14 pixels (the 14 x 14 fallback glyph); type 8 uses char_to_digit_glyph and skips unknown characters; other types count nothing. The stock code keeps the length of an invalid lead byte from the previous character, and for the first character from the caller's r9 (the C starts at 1).", TXT + """
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
""")

wm(0x4002CE7C, 600, "text_draw",
   "Draws the UTF-8 string s at (x, y) in the current font (word 0x42E68C74) and stores the drawn width + 1 in u16 0x42E68BE4. Font type 1: font_char_draw of font_glyph_index, or for unknown characters glyph_14x14_draw of the code from table u16 0x42CF64B4[code] (advance 14). Font type 8: font_char_tint_draw of char_to_digit_glyph (unknown characters skipped). The advance is the glyph width (font + 0x262 + 2 * glyph) plus the spacing u16 0x42E68BE2. With byte 0x42E68BE0 set, each glyph is centred on x (x - width / 2) and the advance is the fixed width u16 font + 8. Same UTF-8 decoding as text_width.", TXT + """
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
""")

NUM = """
/* Formats v with dec decimals into buf: 16 digits are taken (digit 15 is v / 10^15, so it can
   exceed 9), leading zeros above the units digit are dropped. mode 0: '-' when negative; mode 1:
   '-' or '+'; any other mode (-1 included): no sign. dec must be 0..15 (the stock code has room
   for 16 digits only). */
static void number_format(char *buf, long long v, int dec, int mode)
{
    int d[16], neg = 0, top = 0, n, k, p = 0;
    long long p10;
    if (v < 0) {
        v = (long long)(0 - (unsigned long long)v);
        neg = 1;
    }
    d[0] = (int)(v % 10);
    for (k = 1, p10 = 10; k < 15; k++, p10 *= 10)
        d[k] = (int)((v % (p10 * 10)) / p10);
    d[15] = (int)(v / 1000000000000000LL);
    for (k = 15; k >= 0; k--)
        if (d[k]) {
            top = k;
            break;
        }
    n = dec < top ? top : dec;
    for (k = 0; k <= n; k++)
        d[k] += '0';
    if (mode == 0) {
        if (neg)
            buf[p++] = '-';
    } else if (mode == 1) {
        buf[p++] = neg ? '-' : '+';
    }
    for (k = n; k >= dec; k--)
        buf[p++] = (char)d[k];
    if (dec)
        buf[p++] = '.';
    for (k = dec - 1; k >= 0; k--)
        buf[p++] = (char)d[k];
    buf[p] = 0;
}
"""

wm(0x4002D2D4, 1024, "number_text_width",
   "Width in pixels (text_width) of the signed 64-bit value v printed with dec decimals (0..15) and the sign mode (0: '-' only; 1: '-' or '+'; other: none). Digits are taken with 64-bit divisions by powers of ten (__aeabi_ldivmod).", NUM + """
u32 text_width(const char *s);

u32 number_text_width(long long v, u32 dec, u32 mode)
{
    char buf[20];
    number_format(buf, v, (int)dec, (int)mode);
    return text_width(buf);
}
""")

wm(0x4002D82C, 1044, "number_draw",
   "Draws the signed 64-bit value v at (x, y) with text_draw, formatted as in number_text_width: dec decimals (byte) and the sign mode (signed byte: 0 '-' only, 1 '-' or '+', other none).", NUM + """
void text_draw(u32 x, u32 y, const char *s);

void number_draw(u32 x, u32 y, long long v, unsigned char dec, signed char mode)
{
    char buf[20];
    number_format(buf, v, dec, mode);
    text_draw(x, y, buf);
}
""")

wm(0x4002EE78, 624, "glyph_alpha_draw",
   "Draws an alpha-mask glyph g (u16 big-endian width and height, then width * height coverage bytes) at (x, y) on the screen *0x40095498 (stride 1022 pixels). Each coverage a is scaled by 255 - byte 0x42E6865C (when that is not 255), a >= 0xF6 stores the solid colour u16 0x42E68660, a < 3 is skipped, otherwise a is lowered by u16 0x42E68BE6 (when greater) and the colour R, G, B (bytes 0x42E6865D..F) is blended in: over black as a * C / 255, over a pixel as (a * C + (255 - a) * P) / 255 with P widened from RGB565 (v * 8 + 7, or 0; white = 255).", """
#define H16(a) (*(volatile unsigned short *)(a))

void glyph_alpha_draw(u32 x, u32 y, const unsigned char *g)
{
    volatile unsigned char *c = (volatile unsigned char *)0x42E6865Cu;
    u32 w = ((u32)g[0] << 8) | g[1], h = ((u32)g[2] << 8) | g[3];
    int k = 255 - c[0], R = c[1], G = c[2], B = c[3];
    unsigned short solid = H16(0x42E68660u);
    u32 idx = x + y * 1022u;
    const unsigned char *src = g;
    if (h == 0)
        return;
    do {
        if (w) {
            volatile unsigned short *fb = (volatile unsigned short *)*(volatile u32 *)0x40095498u;
            u32 j, p = idx;
            for (j = 0; j < w; j++, p++) {
                unsigned short d = fb[p];
                int a = src[4 + j], r, gg, b;
                if (k != 255)
                    a = (unsigned char)((a * k) / 255);
                if (a >= 0xF6) {
                    fb[p] = solid;
                    continue;
                }
                if (a < 3)
                    continue;
                if ((int)H16(0x42E68BE6u) < a)
                    a = (unsigned char)(a - H16(0x42E68BE6u));
                if (d == 0) {
                    r = a * R / 255;
                    gg = a * G / 255;
                    b = a * B / 255;
                } else {
                    int pr, pg, pb;
                    if (d == 0xFFFF) {
                        pr = pg = pb = 0xFF;
                    } else {
                        pr = (d >> 11) & 31;
                        pg = (d >> 5) & 63;
                        pb = d & 31;
                        if (pr)
                            pr = pr * 8 + 7;
                        if (pg)
                            pg = pg * 4 + 3;
                        if (pb)
                            pb = pb * 8 + 7;
                    }
                    r = (a * R + (255 - a) * (unsigned char)pr) / 255;
                    gg = (a * G + (255 - a) * (unsigned char)pg) / 255;
                    b = (a * B + (255 - a) * (unsigned char)pb) / 255;
                }
                fb[p] = (unsigned short)((((unsigned char)gg >> 2) << 5) | (((unsigned char)r >> 3) << 11) |
                                         ((unsigned char)b >> 3));
            }
        }
        idx += 1022;
        src += w;
    } while (--h);
}
""")
