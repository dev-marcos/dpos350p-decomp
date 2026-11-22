/*
 * manual_cursors_draw -- stock 0x4003117C (1424 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The manual cursors, unless the touch byte 0x42E68BB0+0x17 or SYS+0x652 is set, or SYS+0x28 is set while SYS+0x3C is 0. Byte SYS+0x802 enables the two time cursors H1 / H2 (words SYS+0x808 / SYS+0x80C, x from the trace left edge), byte SYS+0x803 the two level cursors V1 / V2 (words SYS+0x810 / SYS+0x814, y from the top). Each cursor is a dotted grey 0x888888 line (dash 3, period 4); the H cursors get a 17 x 14 marker with a triangle and 'H1' / 'H2' at the top, the V cursors a triangle and 'V1' / 'V2' at the right edge (x = 0x39E .. 0x3B6), kept 8 pixels inside the trace area. With the zoom byte SYS+0x2AA set, the area starts 2 pixels right and a third of its height lower, and the V positions are scaled by 2/3.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void text_draw(u32 x, u32 y, const char *s);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void vline_draw(u32 x, u32 y0, u32 y1);
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define B8(a) (*(volatile unsigned char *)(a))
#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

static void hmark(u32 xc, u32 top, const char *digit, unsigned short ty1, unsigned short ty2, u32 dx2)
{
    u32 y = top + 0xB, j;
    draw_colour_set(0xDDDDDD);
    rect_fill(U16(xc - 8), U16(top), 0x11, 0xE);
    word_42e68c74_set(0x42E65418u);
    for (j = 0; j <= 8; j++) {
        vline_draw(U16(xc - j), U16(y), U16(y + 0xD - j));
        vline_draw(U16(xc + j), U16(y), U16(y + 0xD - j));
    }
    draw_colour_set(0);
    text_draw(U16(xc - 3), ty1, "H");
    text_draw(U16(xc - dx2), ty2, digit);
}

static int clamp_y(int y)
{
    int lo = (int)H16(SYS + 6) + 8, hi = (int)(H16(SYS + 6) + H16(SYS + 0xA)) - 8;
    if (y < lo)
        return lo;
    return hi > y ? y : hi;
}

static void vmark(int pos)
{
    int j;
    draw_colour_set(0xDDDDDD);
    for (j = 0; j <= 8; j++) {
        func_4002ecc4_span(U16(pos - j), U16(0x39E + j), 0x3AA);
        func_4002ecc4_span(U16(pos + j), U16(0x39E + j), 0x3AA);
    }
    rect_fill(0x3AB, U16(pos - 8), 0xB, 0x11);
}

void manual_cursors_draw(void)
{
    u32 x0, y0, right, bottom, h, h1, h2, v1, v2, i;
    int nx, ny, p;
    if (B8(0x42E68BB0u + 0x17))
        return;
    if (B8(SYS + 0x652))
        return;
    if (B8(SYS + 0x28) && B8(SYS + 0x3C) == 0)
        return;
    if (B8(SYS + 0x802) == 0 && B8(SYS + 0x803) == 0)
        return;
    x0 = H16(SYS + 4);
    y0 = H16(SYS + 6);
    h = H16(SYS + 0xA);
    right = x0 + H16(SYS + 8);
    bottom = y0 + h;
    h1 = W32(SYS + 0x808);
    h2 = W32(SYS + 0x80C);
    v1 = W32(SYS + 0x810);
    v2 = W32(SYS + 0x814);
    if (B8(SYS + 0x2AA) == 0) {
        x0 += 1;
        y0 += 2;
    } else {
        x0 += 2;
        y0 = y0 + ((h * 0xAAABu) >> 17) + 3;
        v1 = (u32)((int)(v1 << 1) / 3);
        v2 = (u32)((int)(v2 << 1) / 3);
    }
    nx = (int)(right - x0 + 1) / 4;
    ny = (int)(bottom - y0 + 1) / 4;
    draw_colour_set(0x888888);
    if (B8(SYS + 0x802) && ny > 0) {
        u32 xa = U16(h1 + x0 + 0xFFFF), xb = h2 + x0 + 0xFFFF;
        for (i = 0; (int)i < ny; i++)
            vline_draw(xa, U16(y0 + 4 * i), U16(y0 + 4 * i + 2));
        for (i = 0; (int)i < ny; i++)
            vline_draw(U16(xb), U16(y0 + 4 * i), U16(y0 + 4 * i + 2));
    }
    if (B8(SYS + 0x803) && nx > 0) {
        u32 ya = U16(v1 + y0 + 0xFFFF), yb = v2 + y0 + 0xFFFF;
        for (i = 0; (int)i < nx; i++)
            func_4002ecc4_span(ya, U16(x0 + 4 * i), U16(x0 + 4 * i + 2));
        for (i = 0; (int)i < nx; i++)
            func_4002ecc4_span(U16(yb), U16(x0 + 4 * i), U16(x0 + 4 * i + 2));
    }
    if (B8(SYS + 0x802)) {
        u32 r7 = y0 - 1 + 0xB;
        hmark(h1 + H16(SYS + 4), y0 - 1, "1", U16(r7 - 0xB), U16(r7 + 0xFFFE), 1);
        hmark(h2 + H16(SYS + 4), y0 - 1, "2", U16(r7 - 0xB), U16(r7 + 0xFFFE), 2);
    }
    if (B8(SYS + 0x803)) {
        p = clamp_y((int)(v1 + y0 - 1));
        vmark(p);
        half_42e68be2_set(2);
        word_42e68c74_set(0x42E65418u);
        draw_colour_set(0);
        text_draw(0x3A6, U16(p + 0xFFFB), "V1");
        p = clamp_y((int)(v2 + y0 - 1));
        vmark(p);
        half_42e68be2_set(1);
        word_42e68c74_set(0x42E65418u);
        draw_colour_set(0);
        text_draw(0x3A5, U16(p + 0xFFFB), "V2");
    }
}
