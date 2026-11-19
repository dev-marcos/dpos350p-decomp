# The FFT settings menu (FFT length, waterfall and 3D switches, calibration), the plot grid of the
# main view and the right-hand menu column.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define LAY 0x42E64520u              /* UI layout table */
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define ENT(n) (TXT + 0x1A0 * (n))

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
"""

wr(0x4002BC40, 1744, "fft_settings_menu_draw",
   "Draws the FFT settings menu (layout +0x244..+0x24A). Row 1: image 0x4008BE68, label of text entry 137 ('FFT length') and four option buttons 0x4B x 0x1E at x + 0x14 + 0x4F x i "
   "with the centred texts '4K', '8K', '16K', '32K'; the option equal to the word 0x42E6875C is filled with theme colour +0x10 and has black text, the others theme +0xC and text "
   "colour +0x14; touch records 0x42E61FF8 + 0xBE2 + 0xA x i, a pressed option is shaded with theme +8. Rows 2 and 3: image, label of entry 138 ('Waterfall_display') / 139 "
   "('3D display') and an ON/OFF switch at x + 0x110 for the bytes 0x42E68669 / 0x42E6866A, with full-width touch records +0xC0A / +0xC14. Row 4: image 0x40089BE8, entry 68 "
   "('Calibration'), the arrow icon at x + 0x126 and record +0xC1E.",
   COM + """
#define SEL 0x42E68668u              /* +0xF4 FFT length index, +1 waterfall, +2 3D */

void text_draw(u32 x, u32 y, const char *s);
u32 text_width(const char *s);
void switch_on_off_draw(u32 x, u32 y, u32 on);
void icon_19c7c_draw(u32 x, u32 y);

static void label_row(u32 x, u32 y, u32 img, u32 e)
{
    draw_colour_set(W32(TH + 4));
    draw_text_shadowed((x + 0x14) & 0xFFFF, y & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(W32(TH + 4));
    text_shadow_draw((x + 0x41 + SB8(e + 0x194)) & 0xFFFF, (y + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
}

static void wide_button(u32 x, u32 y, u32 w, u32 rec)
{
    touch_button_set_rect(x, (y - 0x14) & 0xFFFF, w, 0x3C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(W32(TH + 8));
        rect_fill(x, (y - 0x14) & 0xFFFF, w, 0x3C);
    }
}

static const char *len_text(int i)
{
    return i == 0 ? "4K" : i == 1 ? "8K" : i == 2 ? "16K" : "32K";
}

void fft_settings_menu_draw(void)
{
    u32 x = H16(LAY + 0x244), y = H16(LAY + 0x246), w = H16(LAY + 0x248), h = H16(LAY + 0x24A);
    u32 r, xo, tw;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    draw_colour_set(W32(TH));
    rect_fill((x + 0xA) & 0xFFFF, (y + 4) & 0xFFFF, (w - 0x1E) & 0xFFFF, 0x78);
    label_row(x, y + 0x14, 0x4008BE68u, ENT(137));
    r = y + 0x50;
    for (i = 0; i < 4; i++) {
        xo = x + 0x14 + 0x4F * i;
        draw_colour_set(W32(SEL + 0xF4) == (u32)i ? W32(TH + 0x10) : W32(TH + 0xC));
        rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x4B, 0x1E);
        draw_colour_set(W32(SEL + 0xF4) != (u32)i ? W32(TH + 0x14) : 0);
        tw = text_width(len_text(i));
        text_draw((xo + ((0x4B - tw) >> 1)) & 0xFFFF, (r + 8) & 0xFFFF, len_text(i));
        touch_button_set_rect(xo & 0xFFFF, (r - 0xF) & 0xFFFF, 0x4B, 0x3C, (volatile unsigned char *)(BTN + 0xBE2 + 0xA * i));
        if (B8(BTN + 0xBE3 + 0xA * i)) {
            draw_colour_set(W32(TH + 8));
            rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x4B, 0x1E);
        }
    }
    r += 0x48;
    label_row(x, r, 0x4008BFD8u, ENT(138));
    switch_on_off_draw((x + 0x110) & 0xFFFF, r & 0xFFFF, B8(SEL + 1));
    wide_button(x, r, w, 0xC0A);
    r += 0x3C;
    label_row(x, r, 0x4008C148u, ENT(139));
    switch_on_off_draw((x + 0x110) & 0xFFFF, r & 0xFFFF, B8(SEL + 2));
    wide_button(x, r, w, 0xC14);
    word_42e68c74_set(0x42E658D8u);
    r += 0x3C;
    label_row(x, r, 0x40089BE8u, ENT(68));
    icon_19c7c_draw((x + 0x126) & 0xFFFF, r & 0xFFFF);
    wide_button(x, r, w, 0xC1E);
}
""")

wr(0x4002C7AC, 1188, "plot_grid_draw",
   "Draws the plot grid of the main view (x0 = SYS+4, y0 = SYS+6, width SYS+8, height SYS+0xA). With the grid byte SYS+0x655 set it draws, in colour 0x303030, vertical lines "
   "every u16 SYS+0x1C and horizontal lines every u16 SYS+0x20 from the top-left corner, ticks of 5 pixels across the vertical centre line every 12.6 pixels (40 of them; every 4.2 and 8.4 in the upper and lower split parts) and ticks "
   "from centre - 1 to centre + 3 on the horizontal centre line every 19 pixels (50). Then the frame in colour 0x505050. With the split-screen byte SYS+0x2AA set the plot "
   "is cut in an upper part of height h/3 + 1 (line step SYS+0x20 / 3) and a lower part from y0 + h/3 + 1 of height 2h/3 + 1 (line step 2 x SYS+0x20 / 3), each with its "
   "own grid; the upper part gets the frame without its bottom line.",
   COM + """
void vline_draw(u32 x, u32 y0, u32 y1);
void func_4002ecc4_span(u32 y, u32 x0, u32 x1);

/* grid lines and centre ticks of one part (ya..yb, horizontal centre line at cy) */
static void grid_part(int x0, int x1, int cx, int ya, int yb, int cy, u32 xs, u32 ys, float st)
{
    int p, k;
    float f;

    draw_colour_set(0x303030);
    for (p = x0; p <= x1; p += xs)
        vline_draw(p & 0xFFFF, ya & 0xFFFF, yb & 0xFFFF);
    for (p = ya; p <= yb; p += ys)
        func_4002ecc4_span(p & 0xFFFF, x0, x1 & 0xFFFF);
    f = (float)ya;
    for (k = 0; k < 40; k++)
        func_4002ecc4_span((int)(f + (float)k * st) & 0xFFFF, (cx - 2) & 0xFFFF, (cx + 2) & 0xFFFF);
    f = (float)x0;
    for (k = 0; k < 50; k++)
        vline_draw((int)(f + (float)k * 19.0f) & 0xFFFF, (cy + 0xFFFF) & 0xFFFF, (cy + 3) & 0xFFFF);
}

static void frame(int x0, int x1, int ya, int yb)
{
    draw_colour_set(0x505050);
    vline_draw(x0, ya & 0xFFFF, yb & 0xFFFF);
    vline_draw(x1 & 0xFFFF, ya & 0xFFFF, yb & 0xFFFF);
    func_4002ecc4_span(ya & 0xFFFF, x0, x1 & 0xFFFF);
    func_4002ecc4_span(yb & 0xFFFF, x0, x1 & 0xFFFF);
}

void plot_grid_draw(void)
{
    int x0 = H16(SYS + 4), w = H16(SYS + 8);
    int cx = x0 + (w >> 1), x1 = x0 + w;
    u32 xs = H16(SYS + 0x1C);
    int split = B8(SYS + 0x2AA), grid = B8(SYS + 0x655);
    u32 ys = H16(SYS + 0x20);
    int y0 = H16(SYS + 6), h = H16(SYS + 0xA);
    int ya, yb, t;

    if (!split) {
        if (grid)
            grid_part(x0, x1, cx, y0, y0 + h, y0 + (h >> 1), xs, ys, 0x1.933334p+3f);    /* 12.6 */
        frame(x0, x1, y0, y0 + h);
        return;
    }
    yb = y0 + h / 3 + 1;
    if (grid)
        grid_part(x0, x1, cx, y0, yb, y0 + h / 6, xs, ys / 3, 0x1.0ccccep+2f);    /* 4.2, bits 0x40866667 */
    draw_colour_set(0x505050);
    vline_draw(x0, y0, yb & 0xFFFF);
    vline_draw(x1 & 0xFFFF, y0, yb & 0xFFFF);
    func_4002ecc4_span(y0, x0, x1 & 0xFFFF);
    h = H16(SYS + 0xA);
    ya = H16(SYS + 6) + h / 3 + 1;
    t = (2 * h) / 3;
    yb = ya + t + 1;
    if (B8(SYS + 0x655))
        grid_part(x0, x1, cx, ya, yb, ya + t / 2, xs, (2 * (u32)H16(SYS + 0x20)) / 3, 0x1.0ccccep+3f);    /* 8.4, bits 0x41066667 */
    frame(x0, x1, ya, yb);
}
""")

wr(0x4001A708, 1464, "side_menu_column_draw",
   "Draws the right-hand menu column (layout +0x22 / +0x24, 0x40 wide, 0x258 high): eight buttons of about 0x4B at y + 0, 0x4B, 0x96, 0xE1, 0x127, 0x175, 0x1BF, 0x20A, each "
   "with a background (theme SYS+0x3DD53C, or 0x282828 for buttons 4 .. 6), an icon at x + 0x15 (images 0x400857FC, 0x40085AF4, 0x40085DEC, 0x40086CC4, 0x400866D4 or "
   "0x400869CC, 0x40086FBC, 0x400851D4, 0x400854E8) and a touch record 0x42E61FF8 + 0x5C + 0xA x i; a pressed button (record byte +1 == 1) is filled with theme +0x10. "
   "The first icon is red when SYS+0x3C is set and green when not; the fifth uses image 0x400869CC in blue when SYS+0x664 is set, 0x400866D4 in orange when not.",
   COM + """
#define TB (SYS + 0x3DD53C)

/* the pressed shade of one button */
static void pressed(u32 x, u32 y, u32 rec, u32 hp)
{
    if (B8(BTN + rec + 1) == 1) {
        draw_colour_set(W32(TB + 0x10));
        rect_fill(x, y & 0xFFFF, 0x40, hp);
    }
}

static void icon_rec(u32 x, u32 y, u32 yi, u32 img, u32 rec)
{
    draw_text_shadowed((x + 0x13) & 0xFFFF, yi & 0xFFFF, img, 2, 2, 0);
    touch_button_set_rect(x & 0xFFFF, y & 0xFFFF, 0x40, 0x46, (volatile unsigned char *)(BTN + rec));
}

void side_menu_column_draw(void)
{
    u32 x = H16(LAY + 0x22) + 2, y = H16(LAY + 0x24);

    draw_colour_set(W32(TB));
    rect_fill(x & 0xFFFF, y, 0x40, 0x47);
    draw_colour_set(W32(TB));
    rect_fill(x & 0xFFFF, y, 0x40, 0x258);
    draw_colour_set(B8(SYS + 0x3C) ? 0xFF0000 : 0xFF00);
    icon_rec(x, y, y + 0x14, 0x400857FCu, 0x5C);
    pressed(x & 0xFFFF, y, 0x5C, 0x47);

    draw_colour_set(W32(TB));
    rect_fill(x & 0xFFFF, (y + 0x4B) & 0xFFFF, 0x40, 0x47);
    draw_colour_set(0x2288FF);
    draw_colour_set(W32(TB + 4));
    draw_colour_set(0x88FF);
    icon_rec(x, y + 0x4B, y + 0x5F, 0x40085AF4u, 0x66);
    pressed(x & 0xFFFF, y + 0x4B, 0x66, 0x47);

    draw_colour_set(W32(TB));
    rect_fill(x & 0xFFFF, (y + 0x96) & 0xFFFF, 0x40, 0x47);
    draw_colour_set(0x22CC66);
    draw_colour_set(W32(TB + 4));
    icon_rec(x, y + 0x96, y + 0xAA, 0x40085DECu, 0x70);
    pressed(x & 0xFFFF, y + 0x96, 0x70, 0x47);

    draw_colour_set(0x282828);
    rect_fill(x & 0xFFFF, (y + 0xE1) & 0xFFFF, 0x40, 0x46);
    draw_colour_set(0x22CC66);
    draw_colour_set(W32(TB + 4));
    icon_rec(x, y + 0xE1, y + 0xF5, 0x40086CC4u, 0x7A);
    pressed(x & 0xFFFF, y + 0xE1, 0x7A, 0x46);

    draw_colour_set(0x282828);
    rect_fill(x & 0xFFFF, (y + 0x127) & 0xFFFF, 0x40, 0x4E);
    draw_colour_set(0xAAAAAA);
    if (!B8(SYS + 0x664)) {
        draw_colour_set(0xFFAA33);
        draw_text_shadowed((x + 0x14) & 0xFFFF, (y + 0x140) & 0xFFFF, 0x400866D4u, 2, 2, 0);
    } else {
        draw_colour_set(0x33AAFF);
        draw_text_shadowed((x + 0x14) & 0xFFFF, (y + 0x140) & 0xFFFF, 0x400869CCu, 2, 2, 0);
    }
    touch_button_set_rect(x & 0xFFFF, (y + 0x127) & 0xFFFF, 0x40, 0x46, (volatile unsigned char *)(BTN + 0x84));
    pressed(x & 0xFFFF, y + 0x128, 0x84, 0x4B);

    draw_colour_set(0x282828);
    rect_fill(x & 0xFFFF, (y + 0x175) & 0xFFFF, 0x40, 0x46);
    draw_colour_set(0x22CC66);
    draw_colour_set(W32(TB + 4));
    icon_rec(x, y + 0x175, y + 0x189, 0x40086FBCu, 0x8E);
    pressed(x & 0xFFFF, y + 0x174, 0x8E, 0x47);

    draw_colour_set(W32(TB));
    rect_fill(x & 0xFFFF, (y + 0x1BF) & 0xFFFF, 0x40, 0x47);
    draw_colour_set(0x88FF22);
    draw_colour_set(W32(TB + 4));
    icon_rec(x, y + 0x1BF, y + 0x1D6, 0x400851D4u, 0x98);
    pressed(x & 0xFFFF, y + 0x1BF, 0x98, 0x47);

    draw_colour_set(W32(TB));
    rect_fill(x & 0xFFFF, (y + 0x20A) & 0xFFFF, 0x40, 0x48);
    draw_colour_set(0x8822FF);
    draw_colour_set(W32(TB + 4));
    draw_colour_set(0x8800FF);
    icon_rec(x, y + 0x20A, y + 0x221, 0x400854E8u, 0xA2);
    pressed(x & 0xFFFF, y + 0x20A, 0xA2, 0x48);
}
""")
