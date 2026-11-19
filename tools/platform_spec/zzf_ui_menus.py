# UI drawing: the zoom-window marker of the split-screen view (the dimmed areas outside the
# window and its shaded edges) and a three-row menu box (icon, text-table label, arrow icon and
# a touch button per row).
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
"""

wr(0x40042520, 616, "split_window_marker_draw",
   "Unless touch byte 0x42E68BB0+0x17 is set or SYS+0x2AA is 0: takes the zoom window of width H16 SYS+0x24 centred at H16 SYS+0x26 inside the plot (x SYS+4, y SYS+6, width SYS+8, "
   "height SYS+0xA), clamps its edges to [1, w-2] and [0, w-1], dims the plot left and right of it (colour 0xCCFFFFFF, a third of the plot height) and draws each edge as three vertical "
   "lines 0xAAAAAA / 0x555555 / 0x222222 shading inwards. (The left edge is clamped against w-2, not x+w-2.)",
   COM + """
void vline_draw(u32 x, u32 y0, u32 y1);

#define H3 ((H16(SYS + 0xA) * 0xAAABu) >> 17)    /* a third of the plot height */
#define VL(col, x) do { draw_colour_set(col); vline_draw((u32)(x) & 0xFFFF, (H16(SYS + 6) + 1) & 0xFFFF, (H16(SYS + 6) + H3) & 0xFFFF); } while (0)

void split_window_marker_draw(void)
{
    int half, c, lo, hi, w, x4, lo2, hi2;

    if (B8(0x42E68BB0u + 0x17))
        return;
    if (B8(SYS + 0x2AA) == 0)
        return;
    half = H16(SYS + 0x24) >> 1;
    c = H16(SYS + 0x26);
    lo = c - half;
    hi = c + half;
    w = H16(SYS + 8);
    if (lo <= 0)
        lo = 1;
    if (w - 2 < lo)
        lo = w - 2;
    if (w - 1 < hi)
        hi = w - 1;
    if (hi < 0)
        hi = 0;
    draw_colour_set(0xCCFFFFFFu);
    rect_fill((H16(SYS + 4) + 1) & 0xFFFF, (H16(SYS + 6) + 1) & 0xFFFF, (u32)lo & 0xFFFF, H3);
    rect_fill((u32)(hi + H16(SYS + 4)) & 0xFFFF, (H16(SYS + 6) + 1) & 0xFFFF, (u32)(H16(SYS + 8) - hi) & 0xFFFF, H3);

    x4 = H16(SYS + 4);
    w = H16(SYS + 8);
    lo2 = x4 + lo;
    if (lo2 < x4 + 2)
        lo2 = x4 + 2;
    if (w - 2 < lo2)
        lo2 = w - 2;
    hi2 = x4 + hi;
    if (hi2 < x4 + 2)
        hi2 = x4 + 2;
    if (w + x4 - 2 < hi2)
        hi2 = w + x4 - 2;
    VL(0xAAAAAAu, lo2);
    VL(0x555555u, lo2 + 1);
    VL(0x222222u, lo2 + 2);
    VL(0xAAAAAAu, hi2);
    VL(0x555555u, hi2 - 1);
    VL(0x222222u, hi2 - 2);
}
""")

wr(0x4001E8E0, 764, "menu_three_rows_draw",
   "Draws the three-row menu box of layout 0x42E64520 +0xAE..+0xB4: fills it with theme colour SYS+0x3DD550, then for each row (0x3C high) the image 0x4008A038 / 0x4008A1A8 / "
   "0x4008A318, the label of text-table entry 110 + row (with its per-language x / y offsets), the icon 0x40019C7C at the right, and a touch button (records 0x42E61FF8 +0x340 / "
   "+0x34A / +0x354) that is shaded with theme colour +8 while pressed.",
   COM + """
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
void icon_19c7c_draw(u32 x, u32 y);

#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */

void menu_three_rows_draw(void)
{
    u32 x = H16(0x42E64520u + 0xAE), y = H16(0x42E64520u + 0xB0);
    u32 w = H16(0x42E64520u + 0xB2), h = H16(0x42E64520u + 0xB4);
    u32 sl, e;
    volatile unsigned char *rec;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    for (i = 0; i < 3; i++) {
        sl = y + 0x14 + 0x3C * i;
        e = TXT + 0x1A0 * (110 + i);
        rec = (volatile unsigned char *)(0x42E61FF8u + 0x340 + 0xA * i);
        draw_colour_set(W32(TH + 4));
        draw_text_shadowed((x + 0x14) & 0xFFFF, sl & 0xFFFF, 0x4008A038u + 0x170 * i, 2, 2, 0);
        draw_colour_set(W32(TH + 4));
        text_shadow_draw((x + 0x41 + SB8(e + 0x194)) & 0xFFFF, (sl + 3 + SB8(e + 0x195)) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
        icon_19c7c_draw((x + 0x14A) & 0xFFFF, sl & 0xFFFF);
        touch_button_set_rect(x, (sl - 0x14) & 0xFFFF, w, 0x3C, rec);
        if (rec[1]) {
            draw_colour_set(W32(TH + 8));
            rect_fill(x, (sl - 0x14) & 0xFFFF, w, 0x3C);
        }
    }
}
""")

wr(0x40025DB4, 1324, "screenshot_view_frame_draw",
   "Draws the frame of the screenshot viewer window (box 0x42E64520 +0x148..+0x14E): the body becomes touch button 0x42E61FF8+0x80E; a 0x1E-high title bar 0x202020 with a 7-line "
   "dark gradient under it, the image 0x4008A038 and text-table entry 110 (the label of the first row of menu_three_rows_draw), and right-aligned at x+0x3ED the file name built by "
   "screenshot_path_make from the current entry of the list at SYS+0xFA4; a 0x3C-high bottom bar at y+0x21C with a gradient above it, three separators and four touch buttons "
   "(+0x818 / +0x822 / +0x82C / +0x836, shaded with theme colour SYS+0x3DD54C while pressed) labelled with text entries 67, 115, 116, 117 centred in 0xFD. "
   "(The separators and labels use 0x21C - y where the bar uses y + 0x21C, as in the stock code.)",
   COM + """
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void screenshot_path_make(u32 v);
u32 text_width(const char *s);

#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define ENT(n) (TXT + 0x1A0 * (n))

void screenshot_view_frame_draw(void)
{
    u32 x = H16(0x42E64520u + 0x148), y = H16(0x42E64520u + 0x14A);
    u32 w = H16(0x42E64520u + 0x14C), h = H16(0x42E64520u + 0x14E);
    u32 k, e, bx, by, ny, tw;

    touch_button_set_rect(x, (y + 0x1E) & 0xFFFF, w, (h - 0x5A) & 0xFFFF, (volatile unsigned char *)(BTN + 0x80E));
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0x202020);
    rect_fill(x, y, w, 0x1E);
    for (k = 1; k < 8; k++) {
        draw_colour_set(k << 29);
        func_4002ecc4_span((y + 0x1E + k - 1) & 0xFFFF, x, (w + x) & 0xFFFF);
    }
    draw_colour_set(0xDDDDDD);
    draw_text_shadowed((x + 5) & 0xFFFF, (y + 5) & 0xFFFF, 0x4008A038u, 2, 2, 0);
    e = ENT(110);
    text_shadow_draw((x + SB8(e + 0x194) + 0x23) & 0xFFFF, (y + SB8(e + 0x195) + 8) & 0xFFFF, (const char *)W32(e), 1, 2, 0);

    half_42e68be2_set(1);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0x777777);
    screenshot_path_make(H16(SYS + 0xFA6 + 2 * (short)H16(SYS + 0xFA4)));
    tw = text_width((const char *)(SYS + 0x88C1));
    text_shadow_draw((0x3ED + x - tw) & 0xFFFF, (y + 7) & 0xFFFF, (const char *)(SYS + 0x88C1), 2, 2, 0);

    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0x202020);
    by = (y + 0x21C) & 0xFFFF;
    rect_fill(x, by, w, 0x3C);
    for (k = 1; k < 8; k++) {
        draw_colour_set(k << 29);
        func_4002ecc4_span((by - k) & 0xFFFF, x, (w + x) & 0xFFFF);
    }
    ny = 0x21C - y;
    draw_colour_set(0);
    for (k = 1; k < 4; k++)
        rect_fill((x + 0xFF * k) & 0xFFFF, (ny + 0xF) & 0xFFFF, 2, 0x1E);
    for (k = 0; k < 4; k++) {
        bx = k ? x + 0xFF * k + 2 : x;
        touch_button_set_rect(bx & 0xFFFF, ny & 0xFFFF, k ? 0xFD : 0xFF, 0x3C, (volatile unsigned char *)(BTN + 0x818 + 0xA * k));
        if (B8(BTN + 0x818 + 0xA * k + 1)) {
            draw_colour_set(W32(SYS + 0x3DD54C));
            rect_fill(bx & 0xFFFF, ny & 0xFFFF, k ? 0xFD : 0xFF, 0x3C);
        }
    }
    draw_colour_set(0xAAAAAA);
    for (k = 0; k < 4; k++) {
        e = ENT(k ? 114 + k : 67);
        tw = text_width((const char *)W32(e));
        text_shadow_draw((x + 0xFF * k + ((0xFD - tw) >> 1) + SB8(e + 0x194)) & 0xFFFF, (ny + SB8(e + 0x195) + 0x16) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    }
}
""")
