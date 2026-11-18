# Widgets (switch, slider, battery, window frame), fonts (descriptors, glyph drawing), pixel
# blending, a framebuffer copy, key repeat and the trace cursor limits.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wa(addr, size, name, desc, body, src="display"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


DRAW = """void draw_colour_set(u32 c);
void rect_fill(u32 a, u32 b, u32 c, u32 d);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void text_draw(u32 x, u32 y, const char *s);
void pixel_put(u32 x, u32 y);
#define W32(a) (*(volatile u32 *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
"""

# ---------------------------------------------------------------- widgets
COLS = [0x222222, 0xFFFFFF, 0xBBBBBB, 0x222222, 0xFFFFFF, 0x222222, 0xBBBBBB, 0xBBFFFFFF, 0x222222, 0xDDDDDD,
        0xDDFFFFFF, 0x444444, 0x88FF, 0x999999, 0x404040, 0x88FF, 0x555555, 0x888888, 0x888888, 0x88FF, 0x555555,
        0x88FF]
wa(0x40019AF4, 152, "colour_table_3dd530_init",
   "Fills the 22 colour words at SYS+0x3DD530 (used by the widgets below) with fixed values.", """
void colour_table_3dd530_init(void)
{
%s
}
""" % "\n".join("    SYS32(0x3DD530 + 0x%02X) = 0x%Xu;" % (4 * i, c) for i, c in enumerate(COLS)), src="ui")

wa(0x40019B8C, 240, "switch_on_off_draw",
   "Draws an ON/OFF switch at (x, y): a 60 x 28 box, then the 30 x 28 half (right for on, left for off) and the text \"ON\" or \"OFF\" (strings at 0x40019DB0 / 0x40019DB4), with the colours SYS+0x3DD568.. . Font word 0x42E68C74 is set to 0x42E64F58 and half 0x42E68BE2 to 2 for the drawing, and restored after.", DRAW + """
void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);

void switch_on_off_draw(u32 x, u32 y, u32 on)
{
    volatile u32 *t = (volatile u32 *)(0x40544E68u + 0x3DD568u);
    u32 font = W32(0x42E68C74u);
    unsigned short h = H16(0x42E68BE2u);
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E64F58u);
    draw_colour_set(t[0]);
    rect_fill(x, (unsigned short)(y + 2), 0x3C, 0x1C);
    if (on) {
        draw_colour_set(t[1]);
        rect_fill((unsigned short)(x + 0x1E), (unsigned short)(y + 2), 0x1E, 0x1C);
        draw_colour_set(t[3]);
        text_draw((unsigned short)(x + 7), (unsigned short)(y + 10), (const char *)0x40019DB0u);
    } else {
        draw_colour_set(t[2]);
        rect_fill(x, (unsigned short)(y + 2), 0x1E, 0x1C);
        draw_colour_set(t[3]);
        text_draw((unsigned short)(x + 0x23), (unsigned short)(y + 10), (const char *)0x40019DB4u);
    }
    word_42e68c74_set(font);
    half_42e68be2_set((unsigned char)h);
}
""", src="ui")

wa(0x40019C7C, 124, "icon_19c7c_draw",
   "Draws a small icon at (x, y) in colour word 0x409223EC: a 10 x 15 box at (x, y + 3) and a triangle of 8 spans pointing left, rows y + 10 +- k from x + 10 to x + 20 - k.", DRAW + """
void icon_19c7c_draw(u32 x, u32 y)
{
    int k;
    draw_colour_set(W32(0x409223ECu));
    rect_fill(x, (unsigned short)(y + 3), 10, 15);
    for (k = 0; k < 8; k++) {
        func_4002ecc4_span((unsigned short)(y + 10 + k), (unsigned short)(x + 10), (unsigned short)(x + 20 - k));
        func_4002ecc4_span((unsigned short)(y + 10 - k), (unsigned short)(x + 10), (unsigned short)(x + 20 - k));
    }
}
""", src="ui")

wa(0x40019CF8, 184, "slider_draw",
   "Draws a 410-pixel slider at (x, y) for v (0..255): filled part w = v * 410 / 100 (colour SYS+0x3DD57C), the rest (SYS+0x3DD578), and a 20 x 40 knob (SYS+0x3DD580) at x + clamp(w, 10, 400) - 10, y - 12.", DRAW + """
void slider_draw(u32 x, u32 y, unsigned char v)
{
    volatile u32 *t = (volatile u32 *)(0x40544E68u + 0x3DD578u);
    int w = v * 410 / 100, c = w;
    if (c < 10)
        c = 10;
    else if (c > 400)
        c = 400;
    draw_colour_set(t[1]);
    rect_fill(x, y, w, 16);
    draw_colour_set(t[0]);
    rect_fill((unsigned short)(x + w), y, (unsigned short)(410 - w), 16);
    draw_colour_set(t[2]);
    rect_fill((unsigned short)(x + c - 10), (unsigned short)(y - 12), 20, 40);
}
""", src="ui")

wa(0x4001C200, 324, "battery_icon_draw",
   "Draws the battery icon in the rectangle at 0x42E6455E (x, y, w, h): black background, a 13 x 24 outline and the 5 x 2 terminal, a fill of 2 * level (SYS+0x778) pixels, and when SYS+0x779 is set a white charging mark (a bar and a 4-row triangle).", DRAW + """
void rect_outline_draw(u32 a, u32 b, u32 c, u32 d);

void battery_icon_draw(void)
{
    u32 x = H16(0x42E64520u + 0x3E), y = H16(0x42E64520u + 0x40);
    u32 w = H16(0x42E64520u + 0x42), h = H16(0x42E64520u + 0x44), lv;
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    draw_colour_set(0xDDDDDD);
    rect_outline_draw(x, (unsigned short)(y + 2), 13, 24);
    rect_fill((unsigned short)(x + 4), y, 5, 2);
    draw_colour_set(0x88FF);
    lv = SYS8(0x778);
    rect_fill((unsigned short)(x + 2), (unsigned short)(y + 24 - 2 * lv), 9, 2 * lv);
    if (SYS8(0x779) == 0)
        return;
    draw_colour_set(0xFFFFFF);
    rect_fill((unsigned short)(x + 5), (unsigned short)(y + 8), 3, 10);
    func_4002ecc4_span((unsigned short)(y + 0x11), (unsigned short)(x + 3), (unsigned short)(x + 9));
    func_4002ecc4_span((unsigned short)(y + 0x12), (unsigned short)(x + 4), (unsigned short)(x + 8));
    func_4002ecc4_span((unsigned short)(y + 0x13), (unsigned short)(x + 5), (unsigned short)(x + 7));
    func_4002ecc4_span((unsigned short)(y + 0x14), (unsigned short)(x + 6), (unsigned short)(x + 6));
}
""", src="ui")

wa(0x40023D04, 340, "window_frame_23d04_draw",
   "Draws the window whose rectangle is at 0x42E64630 (x, y, w, h): background in colour word 0x409223B8, eight 0x555555 segments (line_draw) near the top and bottom, and registers the rectangle as touch button 0x42E62716.", DRAW + """
void line_draw(u32 a, u32 b, u32 c, u32 d);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);

void window_frame_23d04_draw(void)
{
    u32 x = H16(0x42E64630u), y = H16(0x42E64632u), w = H16(0x42E64634u), h = H16(0x42E64636u), t, u;
    draw_colour_set(W32(0x409223B8u));
    rect_fill(x, y, w, h);
    draw_colour_set(0x555555);
    line_draw(x + 0xB, y + 0x19, x + 0x1F, y + 0xF);
    line_draw(x + 0x34, y + 0x19, x + 0x20, y + 0xF);
    line_draw(x + 0xB, y + 0x1A, x + 0x1F, y + 0x10);
    line_draw(x + 0x34, y + 0x1A, x + 0x20, y + 0x10);
    t = h + y - 0x1B;
    u = h + y - 0x11;
    line_draw(x + 0xB, t, x + 0x1F, u);
    line_draw(x + 0x34, t, x + 0x20, u);
    t = h + y - 0x1C;
    u = h + y - 0x12;
    line_draw(x + 0xB, t, x + 0x1F, u);
    line_draw(x + 0x34, t, x + 0x20, u);
    touch_button_set_rect(x, y, w, h, (volatile unsigned char *)0x42E62716u);
}
""", src="ui")

wa(0x40025D08, 128, "image_42468a58_show",
   "Registers the rectangle at 0x42E6465A as touch button 0x42E627FC, then copies the 1022 x 600 image at 0x42468A58 to the drawing target, rows in reverse order (bottom-up bitmap).", """
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);

void image_42468a58_show(void)
{
    volatile unsigned short *r = (volatile unsigned short *)(0x42E64520u + 0x13A);
    const unsigned short *s = (const unsigned short *)0x42468A58u;
    int i, o = 0;
    touch_button_set_rect(r[0], r[1], r[2], r[3], (volatile unsigned char *)0x42E627FCu);
    for (i = 600; i; i--) {
        copy_u16((unsigned short *)(*(volatile u32 *)0x40095498u + o + 0x12AEA4u), s, 0x3FE);
        s += 0x3FE;
        o -= 0x7FC;
    }
}
""", src="ui")

# ---------------------------------------------------------------- fonts
FONTS = [(0x42E64F58, 1, 0x4007DA1C), (0x42E65418, 1, 0x4007A4EC), (0x42E64A98, 1, 0x40076C98),
         (0x42E658D8, 1, 0x40072D78), (0x42E65D98, 1, 0x4006DDEC), (0x42E66258, 8, 0x400841F0),
         (0x42E66718, 8, 0x400836E8), (0x42E66BD8, 8, 0x4008279C), (0x42E67098, 8, 0x400809D0)]
wa(0x4002CCD8, 236, "fonts_init",
   "Sets up the nine font descriptors (0x4C0 bytes each): byte +0 (1 or 8) and the glyph data pointer +0x4BC (in the graphics blob), then font_metrics_build on each.", """
void font_metrics_build(volatile unsigned char *d);

void fonts_init(void)
{
%s
%s
}
""" % ("\n".join("    *(volatile unsigned char *)0x%08Xu = %d;\n    *(volatile u32 *)0x%08Xu = 0x%08Xu;" % (d, k, d + 0x4BC, g)
                 for d, k, g in FONTS),
       "\n".join("    font_metrics_build((volatile unsigned char *)0x%08Xu);" % d for d, k, g in FONTS)))

wa(0x4002DCAC, 196, "font_metrics_build",
   "Reads a font's glyph data header (big-endian u16 count a and u16 height b at +0x4BC): for each non-zero byte p[i] of the a bytes after the header, stores i + 1 in the u16 table d+0xA.. and, from the second on, the distance to the previous one in d+0x260..; then d+2 = a, d+4 = b, d+6 = number of glyphs - 1, d+8 = largest distance - 1.", """
void font_metrics_build(volatile unsigned char *d)
{
    const unsigned char *p = *(const unsigned char * volatile *)(d + 0x4BC);
    u32 a = p[1] | (p[0] << 8), b = p[3] | (p[2] << 8);
    int end = (unsigned short)(a + 4), i;
    unsigned short n = 0, mx = 0;
    for (i = 4; i < end; i++) {
        if (p[i]) {
            volatile unsigned short *h = (volatile unsigned short *)(d + n * 2);
            h[5] = i + 1;
            if (n) {
                unsigned short v = 0xFFFF + (i + 1) - h[4];
                h[0x130] = v;
                if (mx < v)
                    mx = v;
            }
            n++;
        }
    }
    *(volatile unsigned short *)(d + 2) = a;
    *(volatile unsigned short *)(d + 4) = b;
    *(volatile unsigned short *)(d + 6) = 0xFFFF + n;
    *(volatile unsigned short *)(d + 8) = mx;
    *(const unsigned char * volatile *)(d + 0x4BC) = p;
}
""")

wa(0x4002CDC4, 184, "glyph_14x14_draw",
   "Draws the 14 x 14 glyph of the two-byte code (hi * 94 + lo - 0x4141, 28 bytes each, 2 per row, MSB first) from the font at 0x40000040 with pixel_put at (x, y). The code layout matches GB2312 rows/columns (DERIVED).", DRAW + """
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
""")

wa(0x4002D0D4, 144, "text_shadow_draw",
   "Draws the string s at (x, y) over an n x m block of copies offset by 1..n / 1..m pixels drawn in colour alpha << 24; the drawing colour (word 0x42E68664) is restored before the final copy at (x, y).", DRAW + """
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha)
{
    u32 saved = W32(0x42E68664u), xx, j;
    draw_colour_set((u32)alpha << 24);
    for (xx = (unsigned short)x + 1; n; n--, xx++)
        for (j = 0; j < m; j++)
            text_draw((unsigned short)xx, (unsigned short)(y + 1 + j), s);
    draw_colour_set(saved);
    text_draw((unsigned short)x, y, s);
}
""")

wa(0x4002E6D8, 216, "font_char_draw",
   "Draws glyph ch of the current font (word 0x42E68C74) at (x, y): rows 1 .. height-2 (u16 +4), each row starting at stride (u16 +2) * row + the glyph offset (+0xA + 2ch) in the glyph data, glyph width at +0x262 + 2ch; non-zero bytes become pixels.", DRAW + """
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
""")

wa(0x4002E7B0, 200, "char_to_digit_glyph",
   "Maps '0'..'9' to 0..9, '.' to 10, '+' to 11 and '-' to 12; any other character to 0xFFFF.", """
unsigned short char_to_digit_glyph(u32 c)
{
    switch (c) {
    case '+': return 11;
    case '-': return 12;
    case '.': return 10;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        return c - '0';
    default: return 0xFFFF;
    }
}
""")

wa(0x4002EA88, 232, "pixel_blend",
   "Blends the drawing colour into the pixel (x, y) of the drawing target: each RGB565 channel is widened to 8 bits (v * 8 + 7, or 0), mixed as (A * pixel + (255 - A) * colour) / 255 with A, R, G, B from 0x42E6865C, and packed back.", """
void pixel_blend(u32 x, u32 y)
{
    volatile unsigned short *px = (volatile unsigned short *)(*(volatile u32 *)0x40095498u + (x + y * 1022u) * 2u);
    volatile unsigned char *c = (volatile unsigned char *)0x42E6865Cu;
    int a = c[0], cr = c[1], cg = c[2], cb = c[3], v = *px, r, g, b;
    r = (v >> 11) & 31;
    g = (v >> 5) & 63;
    b = v & 31;
    r = (unsigned char)(r ? r * 8 + 7 : 0);
    g = (unsigned char)(g ? g * 4 + 3 : 0);
    b = (unsigned char)(b ? b * 8 + 7 : 0);
    r = (a * r + (255 - a) * cr) / 255;
    g = (a * g + (255 - a) * cg) / 255;
    b = (a * b + (255 - a) * cb) / 255;
    *px = (((unsigned char)r >> 3) << 11) | (((unsigned char)g >> 2) << 5) | ((unsigned char)b >> 3);
}
""")

wa(0x400342FC, 112, "fb_rect_copy_to_e4e84",
   "dcache_clean_inv_all_t (with the same r0..r3), then copies the rectangle x0 <= x < x1, y0 <= y < y1 (unsigned) of 16-bit pixels, stride 1022, from 0x41FB8FE8 to the buffer 0x420E4E84 (the reverse of fb_rect_copy_from_e4e84).", """
void dcache_clean_inv_all_t(u32 a, u32 b, u32 c, u32 d);

void fb_rect_copy_to_e4e84(u32 x0, u32 y0, u32 x1, u32 y1)
{
    u32 y, n;
    dcache_clean_inv_all_t(x0, y0, x1, y1);
    for (y = y0; y < y1; y++) {
        if (x0 < x1) {
            volatile unsigned short *s = (volatile unsigned short *)(0x41FB8FE8u + (x0 + y * 1022u) * 2u);
            volatile unsigned short *d = (volatile unsigned short *)(0x420E4E84u + (x0 + y * 1022u) * 2u);
            for (n = x1 - x0; n; n--)
                *d++ = *s++;
        }
    }
}
""")

# ---------------------------------------------------------------- key repeat, trace cursors
wa(0x40032D3C, 192, "key_repeat_update",
   "Two repeat counters (u16 SYS+0x86E, +0x870) with request flags SYS+0x872, +0x873: when a flag is set it is cleared and the counter goes up when less than word SYS+0x87C ticks passed since its time stamp (SYS+0x874, +0x878), otherwise down to a minimum of 1; the stamp is then the current tick.", """
#define TICK (*(volatile u32 *)0x42E68CD8u)

void key_repeat_update(void)
{
    int c = SYS16(0x86E), d;
    if (SYS8(0x872)) {
        SYS8(0x872) = 0;
        if (TICK - SYS32(0x874) < SYS32(0x87C))
            c++;
        else if (c >= 2)
            c--;
        SYS32(0x874) = TICK;
    }
    SYS16(0x86E) = c;
    if (SYS8(0x873)) {
        SYS8(0x873) = 0;
        d = SYS16(0x870);
        if (TICK - SYS32(0x878) < SYS32(0x87C))
            SYS16(0x870) = d + 1;
        else if (d >= 2)
            SYS16(0x870) = d - 1;
        SYS32(0x878) = TICK;
    }
}
""", src="ui")

wa(0x40037944, 284, "trace_cursor_limits",
   "Unless touch byte +0x17 is set: limits the values at 0x42E18114/0x42E18118 by the bounds at 0x42E4289C/0x42E428A0, the two widths 0x42E1811C/0x42E18120 to 10 .. word 0x42E42898 (and to each other in modes 3 and 4 of 0x42E18110), 0x42E18124 to 20..500; then passes them to gen_limits through 0x42E42884/0x42E42888 and copies the result back.", """
#define TC32(o) (*(volatile int *)(0x42E180C8u + (o)))
#define Q32(o) (*(volatile int *)(0x42E42884u + (o)))

void trace_cursor_limits(void)
{
    int a = TC32(0x54), b = TC32(0x58), v, c, m;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    v = Q32(0x18);
    c = TC32(0x4C);
    if (v < c)
        TC32(0x4C) = v;
    else if (c < 0)
        TC32(0x4C) = 0;
    v = Q32(0x1C);
    c = TC32(0x50);
    if (v < c)
        TC32(0x50) = v;
    else if (c < -v)
        TC32(0x50) = -v;
    m = Q32(0x14);
    if (m < a)
        a = m;
    else if (a < 10)
        a = 10;
    if (m < b)
        b = m;
    else if (b < 10)
        b = 10;
    if (TC32(0x48) == 3) {
        if (b <= a)
            a = b;
    } else if (TC32(0x48) == 4) {
        if (a >= b)
            b = a;
    }
    v = TC32(0x5C);
    if (v > 500)
        TC32(0x5C) = 500;
    else if (v < 20)
        TC32(0x5C) = 20;
    Q32(0) = TC32(0x4C);
    Q32(4) = TC32(0x50);
    TC32(0x58) = b;
    TC32(0x54) = a;
    gen_limits(a, b, Q32(0), Q32(4));
    TC32(0x4C) = Q32(0);
    TC32(0x50) = Q32(4);
}
""", src="acquisition")

wa(0x40037B48, 224, "trace_defaults_37b48",
   "Unless touch byte +0x17 is set: sets a group of SYS flags and values to their defaults (0x3E, 0x3F, 0x102, 0x103, 0x46, 0x10A, 0x54F, 0x274, 0x275 = 1; words 0x60, 0x120 = 250; 0x2A9 = 0x21; 0x660 = 8; 0x70D pulsed 1 then 0), touch byte +0x16 = 1, 0x42E4287E = 0, 0x42E4288C = 500, and builds the table at 0x42E554B0: start 1,000,000, end 50,000,000, step 200,000, the values from +0x10 and the count at +0xC.", """
#define H32(o) (*(volatile u32 *)(0x42E554B0u + (o)))

void trace_defaults_37b48(void)
{
    u32 i, v;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    SYS8(0x3E) = 1;
    SYS8(0x3F) = 1;
    SYS32(0x60) = 0xFA;
    SYS8(0x102) = 1;
    SYS8(0x103) = 1;
    SYS32(0x120) = 0xFA;
    SYS8(0x46) = 1;
    SYS8(0x10A) = 1;
    SYS8(0x54F) = 1;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 1;
    SYS8(0x2A9) = 0x21;
    SYS8(0x70D) = 1;
    SYS8(0x70D) = 0;
    SYS8(0x660) = 8;
    SYS8(0x275) = 1;
    SYS8(0x274) = 1;
    *(volatile unsigned char *)(0x42E3F984u + 0x2EFA) = 0;
    *(volatile u32 *)(0x42E3F984u + 0x2F08) = 500;
    H32(4) = 50000000;
    H32(0) = 1000000;
    H32(8) = 200000;
    i = 0;
    v = 1000000;
    do {
        H32(0xC) = i;
        H32(0x10 + 4 * i) = v;
        v += H32(8);
        i = H32(0xC) + 1;
    } while (H32(4) >= v);
    H32(0xC) = i;
}
""", src="acquisition")

wa(0x40037AB8, 144, "trace_update_chain",
   "trace_cursor_limits and trace_defaults_37b48, then (depending on the bytes 0x42E1810C/0D/0F) func_4003a300, bode_grid_draw, func_4003963c + func_40039918, and func_40039254 unless +0x44 is set while +0x47 is clear; when +0x44 is 1 and +0x47 is set, +0x45 = 1 and window 0x22 is invalidated. Ends with trace_cursor_values and func_40037c3c_unless_t17.", """
#define G8(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))
void trace_cursor_limits(void);
void trace_defaults_37b48(void);
void func_4003a300(void);
void bode_grid_draw(void);
void func_4003963c(void);
void func_40039918(void);
void func_40039254(void);
void ui_window_invalidate_from(u32 id, u32 r1);
void trace_cursor_values(void);
void func_40037c3c_unless_t17(void);

void trace_update_chain(void)
{
    trace_cursor_limits();
    trace_defaults_37b48();
    if (G8(0x45) == 0)
        func_4003a300();
    bode_grid_draw();
    if (G8(0x47)) {
        if (G8(0x45))
            goto redraw;
        func_4003963c();
        func_40039918();
    }
    if (G8(0x44) && G8(0x47) == 0)
        goto done;
redraw:
    func_40039254();
    if (G8(0x44) == 1) {
        u32 k = G8(0x47);
        if (k) {
            G8(0x45) = 1;
            ui_window_invalidate_from(0x22, k);
        }
    }
done:
    trace_cursor_values();
    func_40037c3c_unless_t17();
}
""", src="acquisition")
