# The display settings menu (waveform brightness, colour temperature, X-Y mode, background grid
# and the entries that open the capture output, frequency-response and spectrum views) and the
# high-resolution mode menu (OFF / 9 .. 16 bit).
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define LAY 0x42E64520u              /* UI layout table */
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed, ... */
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

wr(0x4001CE30, 1916, "display_menu_draw",
   "Draws the display settings menu (layout +0x92..+0x98), seven rows with an image at x + 0x14 and the label of text entry 16 + row. Row 0 'Waveform light': '%' at the right, "
   "the value byte SYS+0x654 as a number (0x4002D724) left of it and a slider 0x40019CF8 for it one row lower, touch record 0x42E61FF8 + 0x20A. Rows 1 .. 3 'Color temperature', "
   "'X-Y mode display', 'Background gird': ON/OFF switches at x + 0x172 for the bytes SYS+0x653, SYS+0x652, SYS+0x655 (grid), records +0x214 / +0x21E / +0x228. Rows 4 .. 6 "
   "'Capture output', 'Frequency response analyzer', 'Spectrum analyzer': the arrow icon at x + 0x186, records +0x232 / +0x23C / +0x246; row 6 also shows ' (CH1)' after its "
   "label. A pressed row (rows 1 .. 6) is shaded with theme colour +8.",
   COM + """
u32 func_4002d2d4_d0(u32 a, u32 b, u32 c);
void text_d6f8_shadow_draw(u32 x, u32 y, u32 a, u32 b, unsigned char e, unsigned char n, unsigned char m, unsigned char alpha);
void slider_draw(u32 x, u32 y, unsigned char v);
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

void display_menu_draw(void)
{
    u32 x = H16(LAY + 0x92), y = H16(LAY + 0x94), w = H16(LAY + 0x96), h = H16(LAY + 0x98);
    u32 r0 = y + 0x14, r, nw;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    label_row(x, r0, 0x40089628u, ENT(16));
    text_shadow_draw((x + w - 0x1C) & 0xFFFF, (r0 + 3) & 0xFFFF, "%", 1, 2, 0);
    half_42e68be2_set(3);
    nw = func_4002d2d4_d0(B8(SYS + 0x654), 0, 0);
    text_d6f8_shadow_draw((x + w - 0x20 - nw) & 0xFFFF, (r0 + 3) & 0xFFFF, B8(SYS + 0x654), 0, 0, 1, 2, 0);
    half_42e68be2_set(1);
    slider_draw((x + 0x14) & 0xFFFF, (r0 + 0x3C) & 0xFFFF, B8(SYS + 0x654));
    touch_button_set_rect(x, (r0 + 0x3C - 0x16) & 0xFFFF, w, 0x3C, (volatile unsigned char *)(BTN + 0x20A));

    r = (r0 + 0x3C + 0x3D) & 0xFFFF;
    label_row(x, r, 0x40089798u, ENT(17));
    switch_on_off_draw((x + 0x172) & 0xFFFF, (r - 5) & 0xFFFF, B8(SYS + 0x653));
    wide_button(x, r, w, 0x214);
    r = (r + 0x3C) & 0xFFFF;
    label_row(x, r, 0x40089908u, ENT(18));
    switch_on_off_draw((x + 0x172) & 0xFFFF, (r - 5) & 0xFFFF, B8(SYS + 0x652));
    wide_button(x, r, w, 0x21E);
    r = (r + 0x3C) & 0xFFFF;
    label_row(x, r, 0x40089A78u, ENT(19));
    switch_on_off_draw((x + 0x172) & 0xFFFF, (r - 5) & 0xFFFF, B8(SYS + 0x655));
    wide_button(x, r, w, 0x228);

    r += 0x3C;
    label_row(x, r, 0x40089D58u, ENT(20));
    icon_19c7c_draw((x + 0x186) & 0xFFFF, r & 0xFFFF);
    wide_button(x, r, w, 0x232);
    r += 0x3C;
    label_row(x, r, 0x4008BB88u, ENT(21));
    icon_19c7c_draw((x + 0x186) & 0xFFFF, r & 0xFFFF);
    wide_button(x, r, w, 0x23C);
    r += 0x3C;
    label_row(x, r, 0x4008BCF8u, ENT(22));
    half_42e68be2_set(2);
    text_shadow_draw((H16(0x42E68BE4u) + x + 0x41) & 0xFFFF, (r + 3) & 0xFFFF, (const char *)0x40090318u, 1, 2, 0);
    half_42e68be2_set(1);
    icon_19c7c_draw((x + 0x186) & 0xFFFF, r & 0xFFFF);
    wide_button(x, r, w, 0x246);
}
""")

wr(0x40021630, 2036, "hires_menu_draw",
   "Draws the high-resolution mode menu (layout +0xE6..+0xEC): image 0x4008C3FC, the label of text entry 36 ('High resolution mode') followed by the text 0x4008F8CC, then nine "
   "option buttons for the byte SYS+0x660: option 0 is 0x19A wide and shows entry 140 ('OFF') above '8 bit'; options 1 .. 8 ('9 bit' .. '16 bit', strings 0x400903D8 + 8 x i) are "
   "0x64 x 0x2D buttons in two rows of four, 0x69 apart. The selected option is filled with theme colour +0x10 and has black text, the others theme +0xC with text colour +0x14. "
   "Touch records 0x42E61FF8 + 0x55C + 0xA x i (0x41 high); a pressed option is shaded with theme +8.",
   COM + """
void text_draw(u32 x, u32 y, const char *s);
u32 text_width(const char *s);

/* fill and text colour of option i */
static void option_colours(int i, u32 *txt)
{
    if (B8(SYS + 0x660) == i) {
        draw_colour_set(W32(TH + 0x10));
        *txt = 0;
    } else {
        draw_colour_set(W32(TH + 0xC));
        *txt = W32(TH + 0x14);
    }
}

void hires_menu_draw(void)
{
    u32 x = H16(LAY + 0xE6), y = H16(LAY + 0xE8), w = H16(LAY + 0xEA), h = H16(LAY + 0xEC);
    u32 e = ENT(36), e2 = ENT(140), r, txt, tw, xo, yb;
    const char *s;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    draw_colour_set(W32(TH + 4));
    draw_text_shadowed((x + 0x14) & 0xFFFF, (y + 0x17) & 0xFFFF, 0x4008C3FCu, 2, 2, 0);
    draw_colour_set(W32(TH + 4));
    text_shadow_draw((x + 0x41 + SB8(e + 0x194)) & 0xFFFF, (y + 0x14 + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    tw = text_width((const char *)W32(e));
    text_shadow_draw((tw + x + SB8(e + 0x194) + 0x49) & 0xFFFF, (y + 0x14 + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)0x4008F8CCu, 1, 2, 0);

    r = (y + 0x50) & 0xFFFF;
    option_colours(0, &txt);
    rect_fill((x + 0x14) & 0xFFFF, r, 0x19A, 0x2D);
    draw_colour_set(txt);
    tw = text_width((const char *)W32(e2));
    text_draw((x + ((0x19A - tw) >> 1) + SB8(e2 + 0x194) + 0x14) & 0xFFFF, (r + SB8(e2 + 0x195) + 6) & 0xFFFF, (const char *)W32(e2));
    tw = text_width((const char *)0x400903D8u);
    text_draw((x + 0x14 + ((0x19A - tw) >> 1)) & 0xFFFF, (r + 0x19) & 0xFFFF, (const char *)0x400903D8u);
    touch_button_set_rect((x + 0x14) & 0xFFFF, (r - 0xA) & 0xFFFF, 0x19A, 0x41, (volatile unsigned char *)(BTN + 0x55C));
    if (B8(BTN + 0x55D)) {
        draw_colour_set(W32(TH + 8));
        rect_fill((x + 0x14) & 0xFFFF, r, 0x19A, 0x2D);
    }

    for (i = 1; i <= 8; i++) {
        if (i == 1 || i == 5)
            r += 0x5A;
        yb = (r - 0xA) & 0xFFFF;
        xo = x + 0x14 + 0x69 * ((i - 1) & 3);
        s = (const char *)(0x400903D8u + 8 * i);
        option_colours(i, &txt);
        rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x64, 0x2D);
        draw_colour_set(txt);
        tw = text_width(s);
        text_draw((xo + ((0x64 - tw) >> 1)) & 0xFFFF, (r + 0xF) & 0xFFFF, s);
        touch_button_set_rect(xo & 0xFFFF, yb, 0x64, 0x41, (volatile unsigned char *)(BTN + 0x55C + 0xA * i));
        if (B8(BTN + 0x55D + 0xA * i)) {
            draw_colour_set(W32(TH + 8));
            rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x64, 0x2D);
        }
    }
}
""")

wr(0x40019DB8, 2304, "main_toolbar_draw",
   "Draws the bottom toolbar of the oscilloscope view (layout +0x14 / +0x16): a black 0x3BA x 0x2C strip, then nine buttons 0x66 x 0x2C, 0x6A apart, labelled with text entries "
   "0 .. 8 ('System', 'Function', 'Measure', 'Browser', 'Cursor X', 'Cursor Y', 'Save wave', 'Screenshot', 'Zoom'). Each has the toolbar theme background SYS+0x3DD530, an icon "
   "0x40088208 + 0x170 x i at x + 0x2A (its own colour is set and at once replaced by theme +4, as in the other toolbars), the label centred in theme colour +8 and the touch "
   "record 0x42E61FF8 + 2 + 0xA x i; a pressed button is filled with theme +0x1C. The last button's background and shade are 0x67 wide.",
   COM + """
#define TT (SYS + 0x3DD530)          /* toolbar theme: +0 fill, +4 icon, +8 text, +0x1C pressed */

u32 text_width(const char *s);

static u32 icon_colour(int i)
{
    switch (i) {
    case 0: case 6: return 0xAA8855;
    case 1: case 7: return 0x5588AA;
    case 2: return 0x88AA55;
    case 3: case 8: return 0x55AA88;
    case 4: return 0xAA5588;
    default: return 0x8855AA;
    }
}

void main_toolbar_draw(void)
{
    u32 x = H16(LAY + 0x14), y = H16(LAY + 0x16), bx, e, tw, wd;
    int i;

    draw_colour_set(0xFFFFFF);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    draw_colour_set(0);
    rect_fill(x, y, 0x3BA, 0x2C);
    for (i = 0; i < 9; i++) {
        bx = x + 0x6A * i;
        wd = i == 8 ? 0x67 : 0x66;
        e = ENT(i);
        draw_colour_set(W32(TT));
        rect_fill(bx & 0xFFFF, y, wd, 0x2C);
        draw_colour_set(icon_colour(i));
        draw_colour_set(W32(TT + 4));
        draw_text_shadowed((bx + 0x2A) & 0xFFFF, (y + 2) & 0xFFFF, 0x40088208u + 0x170 * i, 2, 2, 0);
        draw_colour_set(W32(TT + 8));
        tw = text_width((const char *)W32(e));
        text_shadow_draw((bx + ((0x66 - tw) >> 1) + SB8(e + 0x194)) & 0xFFFF, (y + SB8(e + 0x195) + 0x19) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
        touch_button_set_rect(bx & 0xFFFF, y, 0x66, 0x2C, (volatile unsigned char *)(BTN + 2 + 0xA * i));
        if (B8(BTN + 3 + 0xA * i)) {
            draw_colour_set(W32(TT + 0x1C));
            rect_fill(bx & 0xFFFF, y, wd, 0x2C);
        }
    }
}
""")
