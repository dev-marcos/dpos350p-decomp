/*
 * time_cursors_draw -- stock 0x40028364 (968 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The two time cursors, when the byte SYS+0x3DA52C is set: positions are the words SYS+0x3DA534 (C1) and SYS+0x3DA538 (C2) from the trace left edge SYS+4. Each gets a dotted vertical line (dash 3, period 4) and a 17 x 14 marker with a triangle and the label 'C1' / 'C2' drawn one character above the other (font 0x42E65418). A bar of the dark colour between them carries the label 'T-CH1' or 'T-CH2' in a 60 x 24 box at mid height. The byte SYS+0x3DA530 picks the channel colours: 0 is 0xBB6611 / 0xFF8822, otherwise 0x1166BB / 0x2288FF.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
u32 text_width(const char *s);
void text_draw(u32 x, u32 y, const char *s);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void vline_draw(u32 x, u32 y0, u32 y1);
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define B8(a) (*(volatile unsigned char *)(a))
#define SYS 0x40544E68u
#define CUR (SYS + 0x3DA52Cu)
#define U16(v) ((unsigned short)(v))

static void marker(u32 xc, u32 top, const char *digit_y_text, unsigned short ty1, unsigned short ty2,
                   u32 col, u32 dx2)
{
    u32 y = top - 1 + 0xB, j;
    draw_colour_set(col);
    rect_fill(U16(xc - 8), U16(top - 1), 0x11, 0xE);
    word_42e68c74_set(0x42E65418u);
    for (j = 0; j < 9; j++) {
        vline_draw(U16(xc - j), U16(y), U16(y + 0xD - j));
        vline_draw(U16(xc + j), U16(y), U16(y + 0xD - j));
    }
    draw_colour_set(0);
    text_draw(U16(xc - 3), ty1, "C");
    text_draw(U16(xc - dx2), ty2, digit_y_text);
}

void time_cursors_draw(void)
{
    u32 left1, top, bottom, c1, c2, dark, light, i, xa, xb, r5;
    int mid, n, half;
    const char *s;
    if (B8(CUR) == 0)
        return;
    left1 = H16(SYS + 4) + 1;
    top = H16(SYS + 6) + 2;
    bottom = H16(SYS + 6) + H16(SYS + 0xA);
    c1 = W32(CUR + 8);
    c2 = W32(CUR + 0xC);
    if (B8(CUR + 4) == 0) {
        dark = 0xBB6611;
        light = 0xFF8822;
    } else {
        dark = 0x1166BB;
        light = 0x2288FF;
    }
    mid = (int)(bottom + top) / 2;
    n = (int)(bottom - top + 1) / 4;
    draw_colour_set(dark);
    if (n > 0) {
        xa = U16(c1 + left1 + 0xFFFF);
        for (i = 0; (int)i < n; i++)
            vline_draw(xa, U16(top + 4 * i), U16(top + 4 * i + 2));
        xb = c2 + left1 + 0xFFFF;
        for (i = 0; (int)i < n; i++)
            vline_draw(U16(xb), U16(top + 4 * i), U16(top + 4 * i + 2));
    }
    r5 = top + 10;
    marker(c1 + H16(SYS + 4), top, "1", U16(r5 - 0xB), U16(r5 + 0xFFFE), light, 1);
    marker(c2 + H16(SYS + 4), top, "2", U16(r5 - 0xB), U16(r5 + 0xFFFE), light, 2);
    half = (int)(c2 - c1 - 0x3C) / 2;
    draw_colour_set(dark);
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    func_4002ecc4_span(U16(mid), U16(c1 + left1), U16(half + c1 + left1));
    xb = U16(c2 + left1 - 1);
    func_4002ecc4_span(U16(mid), U16(xb - half), xb);
    draw_colour_set(dark);
    rect_fill(U16(half + c1 + left1), U16(mid - 0xC), 0x3C, 0x18);
    draw_colour_set(0);
    s = B8(CUR + 4) == 0 ? (const char *)0x40090420u : (const char *)0x40090428u;
    text_draw(U16(half + c1 + left1 + ((0x3C - text_width(s)) >> 1)), U16(mid - 7), s);
}
