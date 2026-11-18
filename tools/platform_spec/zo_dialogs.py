# Small dialogs and screen pieces: the "Delete selected N items ?" dialogs, the notice / warning
# message boxes with a Confirm button, the four colour buttons, the XY-mode grid and the two
# trace markers. Strings come from the UI text table 0x42C2FE74 (entries of 0x1A0 bytes; the
# English texts quoted below were read by running the stock filler 0x40045974 in the emulator).
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wo(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


UI = """void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
u32 text_width(const char *s);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define WR16(o) H16(0x42E64520u + (o))
#define BTN ((volatile unsigned char *)0x42E61FF8u)
#define TXE(k) (0x42C2FE74u + 0x1A0u * (k))
#define TXS(k) ((const char *)W32(TXE(k)))
#define DX(k) (*(volatile signed char *)(TXE(k) + 0x194))
#define DY(k) (*(volatile signed char *)(TXE(k) + 0x195))
#define U16(v) ((unsigned short)(v))
"""

DEL = UI + """void text_d6f8_shadow_draw(u32 x, u32 y, u32 a, u32 b, unsigned char e, unsigned char n, unsigned char m,
                           unsigned char alpha);
#define COL(i) W32(0x40544E68u + 0x3DD54Cu + 4 * (i))

/* "Delete selected" <count SYS+0x88BE> "items" " ?" over a box at rectangle roff of the window
   table 0x42E64520, then a 240 x 50 button labelled "Confirm" (text 95) with touch slot boff. */
static void delete_dialog(u32 roff, u32 boff, int dx_in_x2)
{
    u32 x = WR16(roff), y = WR16(roff + 2), w = WR16(roff + 4), h = WR16(roff + 6);
    u32 cx = x + (w >> 1), ty = y + 0x14, x1, x2, x3, bx, by, tw;
    draw_colour_set(COL(1));
    rect_fill(x, y, w, h);
    draw_colour_set(COL(2));
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E658D8u);
    x1 = U16(x + 0x38);
    text_shadow_draw(U16(x1 + DX(118)), U16(ty + DY(118)), TXS(118), 1, 2, 0);
    x2 = U16(H16(0x42E68BE4u) + x1 + (dx_in_x2 ? DX(118) : 0) + 8);
    text_d6f8_shadow_draw(x2, U16(ty), H16(0x40544E68u + 0x88BE), 0, 0, 1, 2, 0);
    x3 = U16(H16(0x42E68BE4u) + x2 + 8);
    text_shadow_draw(U16(x3 + DX(120)), U16(ty + DY(120)), TXS(120), 1, 2, 0);
    text_shadow_draw(U16(H16(0x42E68BE4u) + x3), U16(ty), " ?", 1, 2, 0);
    bx = cx - 0x78;
    by = y + 0x50;
    draw_colour_set(COL(4));
    rect_fill(U16(bx), U16(by), 0xF0, 0x32);
    touch_button_set_rect(U16(bx), U16(by - 10), 0xF0, 0x46, BTN + boff);
    if (BTN[boff + 1]) {
        draw_colour_set(COL(0));
        rect_fill(U16(bx), U16(by), 0xF0, 0x32);
    }
    draw_colour_set(COL(2));
    tw = text_width(TXS(95));
    text_shadow_draw(U16(x + ((w - tw) >> 1) + DX(95)), U16(by + DY(95) + 0x14), TXS(95), 1, 2, 0);
}
"""

for addr, size, name, roff, boff, dxx in [(0x400262F8, 664, "ui_delete_confirm_164", 0x164, 0x912, 0),
                                         (0x400271B0, 664, "ui_delete_confirm_180", 0x180, 0x9EE, 0),
                                         (0x40024E38, 656, "ui_delete_confirm_12c", 0x12C, 0x7FA, 1)]:
    wo(addr, size, name,
       "Dialog 'Delete selected <n> items ?' (n = u16 SYS+0x88BE) in the box at window rectangle 0x42E64520+0x%X, with the 'Confirm' button (touch slot 0x42E61FF8+0x%X, drawn pressed when its byte +1 is set). Colours from SYS+0x3DD54C.%s" % (roff, boff, " Unlike the other two copies, the count is placed after the x offset of the first text as well." if dxx else ""),
       DEL + """
void %s(void)
{
    delete_dialog(0x%X, 0x%X, %d);
}
""" % (name, roff, boff, dxx))

MSG = UI + """#define COL(i) W32(0x40544E68u + 0x3DD550u + 4 * (i))

/* Message box at rectangle roff: title text t centred, three lines b..b+2 at x offset xo, and a
   'Confirm' button (text 95) bd pixels lower than usual, touch slot boff. */
static void msgbox(u32 roff, u32 t, u32 b, u32 xo, u32 bd, u32 boff)
{
    u32 x = WR16(roff), y = WR16(roff + 2), w = WR16(roff + 4), h = WR16(roff + 6);
    u32 iw, x2, y2, bx, bw, by, tw;
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    draw_colour_set(COL(0));
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    iw = U16(w - 2);
    x2 = x + 2;
    y2 = y + 0x14;
    draw_colour_set(COL(1));
    tw = text_width(TXS(t));
    text_shadow_draw(U16(x2 + ((iw - tw) >> 1) + DX(t)), U16(y2 + DY(t) + 3), TXS(t), 1, 2, 0);
    text_shadow_draw(U16(x2 + DX(b) + xo), U16(y2 + DY(b) + 0x23), TXS(b), 1, 2, 0);
    text_shadow_draw(U16(x2 + DX(b + 1) + xo), U16(y2 + DY(b + 1) + 0x39), TXS(b + 1), 1, 2, 0);
    text_shadow_draw(U16(x2 + DX(b + 2) + xo), U16(y2 + DY(b + 2) + 0x4F), TXS(b + 2), 1, 2, 0);
    bx = x2 + 0x14;
    bw = U16(iw - 0x28);
    by = y2 + 0x6E + bd;
    draw_colour_set(COL(3));
    rect_fill(U16(bx), U16(by), bw, 0x28);
    draw_colour_set(COL(1));
    tw = text_width(TXS(95));
    text_shadow_draw(U16(x2 + ((iw - tw) >> 1) + DX(95)), U16(y2 + DY(95) + 0x7A + bd), TXS(95), 1, 2, 0);
    touch_button_set_rect(U16(bx), U16(y2 + 0x64 + bd), bw, 0x3C, BTN + boff);
    if (BTN[boff + 1]) {
        draw_colour_set(COL(2));
        rect_fill(U16(bx), U16(by), bw, 0x28);
    }
}
"""

for addr, size, name, roff, t, b, xo, bd, boff, what in [
        (0x4002903C, 660, "ui_unplug_notice_19c", 0x19C, 91, 92, 0x39, 0, 0x188,
         "'Notice !' / 'Please unplug all signal' 'proble and USB first !'"),
        (0x4002C32C, 664, "ui_unplug_notice_252", 0x252, 91, 92, 0x39, 0, 0xC28,
         "'Notice !' / 'Please unplug all signal' 'proble and USB first !'"),
        (0x4002967C, 668, "ui_factory_reset_confirm", 0x1AA, 91, 102, 0x31, 0, 0x1EC,
         "'Notice !' / 'Restore to factory' 'settings ?'"),
        (0x40029920, 672, "ui_clear_data_warning", 0x1AA, 106, 107, 0x31, 0, 0x200,
         "'Warning !' / 'All saved data will' 'be clear !'"),
        (0x40029EC0, 672, "ui_connect_generator_notice", 0x20C, 91, 126, 0x28, 5, 0xB4C,
         "'Notice !' / 'Please connect all channels' 'to the signal generator !'")]:
    wo(addr, size, name,
       "Message box %s (texts %d and %d..%d of the UI table) in the box at window rectangle 0x42E64520+0x%X, with the 'Confirm' button (text 95, touch slot 0x42E61FF8+0x%X, drawn pressed when its byte +1 is set). Colours from SYS+0x3DD550." % (what, t, b, b + 2, roff, boff),
       MSG + """
void %s(void)
{
    msgbox(0x%X, %d, %d, 0x%X, %d, 0x%X);
}
""" % (name, roff, t, b, xo, bd, boff))

wo(0x40028084, 664, "ui_colour_buttons_draw",
   "Column of four 64 x 150 buttons at window rectangle 0x42E64520+0x18E (x + 2), each with a shadowed alpha image (0x400860E4, 0x400863DC, 0x400866D4, 0x400869CC) in its own colour (0xFF2288, 0x88FF22, 0xFF8822, 0x2288FF) and touch slots 0x42E61FF8+0x9F8/0xA02/0xA0C/0xA16. A slot whose byte +1 is exactly 1 is filled with colour SYS+0x3DD54C (pressed). Background colour SYS+0x3DD53C.", UI + """
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
#define COL(i) W32(0x40544E68u + 0x3DD53Cu + 4 * (i))

void ui_colour_buttons_draw(void)
{
    static const u32 img[4] = {0x400860E4u, 0x400863DCu, 0x400866D4u, 0x400869CCu};
    static const u32 ink[4] = {0xFF2288u, 0x88FF22u, 0xFF8822u, 0x2288FFu};
    u32 x = U16(WR16(0x18E) + 2), y = WR16(0x190), w = WR16(0x192), h = WR16(0x194), i;
    u32 tx = x + 0x14;
    draw_colour_set(COL(0));
    rect_fill(x, y, w, h);
    for (i = 0; i < 4; i++) {
        if (i >= 2) {
            draw_colour_set(COL(0));
            rect_fill(x, U16(y), 0x40, 0x97);
        }
        draw_colour_set(ink[i]);
        draw_text_shadowed(U16(tx), U16(y + 0x3D), img[i], 2, 2, 0);
        touch_button_set_rect(x, U16(y), 0x40, 0x96, BTN + 0x9F8 + 10 * i);
        if (BTN[0x9F9 + 10 * i] == 1) {
            draw_colour_set(COL(4));
            rect_fill(x, U16(y), 0x40, 0x96);
        }
        y += 0x96;
    }
}
""")

wo(0x4002C5F8, 416, "xy_grid_draw",
   "Grid of the XY display: a square of side H = u16 SYS+0xA centred horizontally in the trace area (x = SYS+4 + (SYS+8 - H) / 2, y = SYS+6). When the byte SYS+0x655 is set, grid lines every u16 SYS+0x20 pixels in 0x303030 and 40 ticks 12.6 pixels apart on the two centre lines; then the border in 0x505050.", """
void draw_colour_set(u32 c);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void vline_draw(u32 x, u32 y0, u32 y1);
#define H16(a) (*(volatile unsigned short *)(a))
#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

void xy_grid_draw(void)
{
    u32 side = H16(SYS + 0xA), top = H16(SYS + 6), step;
    int left = (int)H16(SYS + 4) + ((int)(H16(SYS + 8) - side) / 2);
    int right = (int)side + left, bottom = (int)(side + top), cy = (int)(top + (side >> 1));
    int cx = left + (int)(side >> 1) + 1, p, i;
    float base;
    step = H16(SYS + 0x20);
    if (*(volatile unsigned char *)(SYS + 0x655)) {
        draw_colour_set(0x303030);
        for (p = left; p <= right; p += step)
            vline_draw(U16(p), top, U16(bottom));
        for (p = (int)top; p <= bottom; p += step)
            func_4002ecc4_span(U16(p), U16(left), U16(right));
        base = (float)(int)top;
        for (i = 0; i < 40; i++)
            func_4002ecc4_span(U16((int)(base + (float)i * 12.6f)), U16(cx - 2), U16(cx + 2));
        base = (float)left;
        for (i = 0; i < 40; i++)
            vline_draw(U16((int)(base + (float)i * 12.6f)), U16(cy - 1), U16(cy + 3));
    }
    draw_colour_set(0x505050);
    vline_draw(U16(left), top, U16(bottom));
    vline_draw(U16(right), top, U16(bottom));
    func_4002ecc4_span(top, U16(left), U16(right));
    func_4002ecc4_span(U16(bottom), U16(left), U16(right));
}
""", src="trace")

wo(0x40024C60, 436, "trace_markers_draw",
   "The two channel markers: '2' in a white 11 x 15 box at the left of the trace area at height SYS+6 + SYS+0xA - word SYS+0x120 - 1 (kept between SYS+6 + 8 and SYS+6 + SYS+0xA - 8), with a white triangle; and '1' in a yellow 15 x 11 box at the top, at x = word SYS+0x60 + the left edge of the centred square + 1, with a triangle drawn by vertical lines. Font 0x42E658D8.", """
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void word_42e68c74_set(u32 a);
void text_draw(u32 x, u32 y, const char *s);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void vline_draw(u32 x, u32 y0, u32 y1);
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

void trace_markers_draw(void)
{
    int x = (int)(H16(SYS + 4) + (H16(SYS + 8) >> 1)) - (int)(H16(SYS + 0xA) >> 1);
    int bot = (int)(H16(SYS + 6) + H16(SYS + 0xA));
    int lo = (int)H16(SYS + 6) + 8, y = bot - (int)W32(SYS + 0x120) - 1, i, k;
    if (y >= lo)
        lo = (bot - 8 > y) ? y : bot - 8;
    draw_colour_set(0xFFFF);
    rect_fill(U16(x), U16(lo - 7), 0xB, 0xF);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0);
    text_draw(U16(x + 3), U16(lo - 6), "2");
    draw_colour_set(0xFFFF);
    x += 0xB;
    for (i = 0; i < 8; i++) {
        func_4002ecc4_span(U16(lo - i), U16(x), U16(x + 7 - i));
        func_4002ecc4_span(U16(lo + i), U16(x), U16(x + 7 - i));
    }
    x = (int)W32(SYS + 0x60) + ((int)(H16(SYS + 4) + (H16(SYS + 8) >> 1)) - (int)(H16(SYS + 0xA) >> 1)) + 1;
    k = (int)H16(SYS + 6) + 1;
    draw_colour_set(0xFFFF00);
    rect_fill(U16(x - 7), U16(k), 0xF, 0xB);
    word_42e68c74_set(0x42E658D8u);
    k += 0xB;
    for (i = 0; i < 8; i++) {
        vline_draw(U16(x - i), U16(k), U16(k + 7 - i));
        vline_draw(U16(x + i), U16(k), U16(k + 7 - i));
    }
    draw_colour_set(0);
    text_draw(U16(x + 0xFFFF), U16(k - 0xB), "1");
}
""", src="trace")

wo(0x40029BC0, 760, "ui_usb_connect_screen",
   "The 'USB connect' screen (text 11) in the window rectangle 0x42E64520+0x1C6: clears the frame buffer, a 0x252525 title bar with the image 0x40088EF8, a 0x252525 'Return' bar at the bottom (text 67, touch slot 0x42E61FF8+0x19C, drawn pressed when its byte +1 is set), then a fixed picture of grey boxes and the label 'USB' centred over x = 240..760. Colours from SYS+0x3DD54C.", UI + """
void fb_clear_current(void);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
#define COL(i) W32(0x40544E68u + 0x3DD54Cu + 4 * (i))

void ui_usb_connect_screen(void)
{
    u32 x = WR16(0x1C6), y = WR16(0x1C8), w = WR16(0x1CA), h = WR16(0x1CC), by, tw;
    const char *usb = "USB";
    fb_clear_current();
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(COL(1));
    draw_colour_set(0x252525);
    rect_fill(x, y, w, 0x1E);
    draw_colour_set(COL(2));
    draw_text_shadowed(U16(x + 5), U16(y + 5), 0x40088EF8u, 2, 2, 0);
    text_shadow_draw(U16(x + DX(11) + 0x23), U16(y + DY(11) + 8), TXS(11), 1, 2, 0);
    by = y + h - 0x32;
    draw_colour_set(COL(1));
    draw_colour_set(0x252525);
    rect_fill(x, U16(by), w, 0x32);
    draw_colour_set(COL(2));
    tw = text_width(TXS(67));
    text_shadow_draw(U16(x + ((w - tw) >> 1) + DX(67)), U16(by + DY(67) + 0xF), TXS(67), 1, 2, 0);
    touch_button_set_rect(x, U16(by), w, 0x32, BTN + 0x19C);
    if (BTN[0x19D]) {
        draw_colour_set(COL(0));
        rect_fill(x, U16(by), w, 0x32);
    }
    draw_colour_set(0xBBBBBB);
    rect_fill(0xF0, 0x69, 0x208, 0x140);
    func_4002ecc4_span(0x68, 0xF1, 0x2F6);
    func_4002ecc4_span(0x1A9, 0xF1, 0x2F6);
    draw_colour_set(0x999999);
    rect_fill(0xF0, 0x69, 0x208, 0x124);
    func_4002ecc4_span(0x68, 0xF1, 0x2F6);
    draw_colour_set(0x777777);
    rect_fill(0x1CC, 0x1AA, 0x50, 0x37);
    rect_fill(0x190, 0x1E1, 0xC8, 3);
    draw_colour_set(0x181818);
    rect_fill(0xF1, 0x6C, 0x206, 0x121);
    draw_colour_set(0xBBBBBB);
    tw = text_width(usb);
    text_shadow_draw(U16(0xF0 + ((0x208 - tw) >> 1)), 0xF2, usb, 1, 2, 0);
}
""")

wo(0x400292E4, 900, "ui_language_menu_draw",
   "Language menu in the window rectangle 0x42E64520+0x84: four rows 60 pixels apart, each with the image 0x400891D8 and the name of a language (the strings at 0x40090430, 0x40090438, 0x4008E7B4, 0x4008FF6C: Chinese, 'English', Russian, 'Portugues' with accent), touch slots 0x42E61FF8+0x156 + 10 * i. A row is drawn selected when its button byte +1 is set or when the byte SYS+0x7AC (menu language, 1..4) is i + 1. Colours from SYS+0x3DD550.", UI + """
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
#define COL(i) W32(0x40544E68u + 0x3DD550u + 4 * (i))

void ui_language_menu_draw(void)
{
    static const u32 name[4] = {0x40090430u, 0x40090438u, 0x4008E7B4u, 0x4008FF6Cu};
    u32 x = WR16(0x84), y = WR16(0x86), w = WR16(0x88), h = WR16(0x8A), bx = x + 2, row, i;
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(COL(0));
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    for (i = 0, row = y + 0x14; i < 4; i++, row += 0x3C) {
        draw_colour_set(COL(1));
        draw_text_shadowed(U16(bx + 0x14), U16(row), 0x400891D8u, 2, 2, 0);
        draw_colour_set(COL(1));
        text_shadow_draw(U16(bx + 0x41), U16(row + 3), (const char *)name[i], 1, 2, 0);
        touch_button_set_rect(U16(bx), U16(row - 0x14), w, 0x3C, BTN + 0x156 + 10 * i);
        if (BTN[0x157 + 10 * i] || *(volatile unsigned char *)(0x40544E68u + 0x7AC) == i + 1) {
            draw_colour_set(COL(2));
            rect_fill(U16(bx), U16(row - 0x14), w, 0x3C);
        }
    }
}
""")

wo(0x40028364, 968, "time_cursors_draw",
   "The two time cursors, when the byte SYS+0x3DA52C is set: positions are the words SYS+0x3DA534 (C1) and SYS+0x3DA538 (C2) from the trace left edge SYS+4. Each gets a dotted vertical line (dash 3, period 4) and a 17 x 14 marker with a triangle and the label 'C1' / 'C2' drawn one character above the other (font 0x42E65418). A bar of the dark colour between them carries the label 'T-CH1' or 'T-CH2' in a 60 x 24 box at mid height. The byte SYS+0x3DA530 picks the channel colours: 0 is 0xBB6611 / 0xFF8822, otherwise 0x1166BB / 0x2288FF.", """
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
""", src="trace")
