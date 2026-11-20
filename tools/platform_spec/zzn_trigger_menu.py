# The trigger settings menu (mode, edge, channel, HF suppression) and the plot edge markers
# (channel offsets, trigger level, trigger position).
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
void text_draw(u32 x, u32 y, const char *s);
u32 text_width(const char *s);
"""

wr(0x40021E68, 2724, "trigger_menu_draw",
   "Draws the trigger menu (layout +0xF4..+0xFA) in four sections, each an image, a text-table label and a row of 0x1E high option buttons: 'Trigger mode' (text entry 37) "
   "with 'Auto', 'Single', 'Normal' (entries 41 .. 43, 0x86 wide) for the byte SYS+0x27A; 'Trigger edge' (38) with 'Rising', 'Falling' (44, 45, 0xCB wide) for SYS+0x27B; "
   "'Trigger channel' (39) with 'CH1', 'CH2' for SYS+0x27C; 'HF suppresstion' (40) with 'OFF' (entry 140), 'L1', 'L2', 'L3' for SYS+0x284. The selected option is filled "
   "with theme colour +0x10 and black text, the others theme +0xC with text colour +0x14; a pressed option is shaded with theme +8. Touch records 0x42E61FF8 + 0x5B6, "
   "+0x5C0, +0x5CA, +0x5D4, +0x5DE, +0x5E8, +0x5F2, +0x5FC, +0x606, +0x610, +0x61A.",
   COM + """
static void label_row(u32 x, u32 y, u32 img, u32 e)
{
    draw_colour_set(W32(TH + 4));
    draw_text_shadowed((x + 0x14) & 0xFFFF, y & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(W32(TH + 4));
    text_shadow_draw((x + 0x41 + SB8(e + 0x194)) & 0xFFFF, (y + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
}

/* one option button: selected when the setting byte at sel equals v; the text is centred, then moved
   by dx / dy (the text-table offsets, 0 for the in-code texts) */
static void option(u32 xo, u32 r, u32 wd, u32 sel, u32 v, const char *s, int dx, int dy, u32 rec)
{
    u32 txt, tw;

    if (B8(sel) == v) {
        draw_colour_set(W32(TH + 0x10));
        txt = 0;
    } else {
        draw_colour_set(W32(TH + 0xC));
        txt = W32(TH + 0x14);
    }
    rect_fill(xo & 0xFFFF, r & 0xFFFF, wd, 0x1E);
    draw_colour_set(txt);
    tw = text_width(s);
    text_draw((xo + ((wd - tw) >> 1) + dx) & 0xFFFF, (r + dy + 8) & 0xFFFF, s);
    touch_button_set_rect(xo & 0xFFFF, (r - 0xF) & 0xFFFF, wd, 0x3C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(W32(TH + 8));
        rect_fill(xo & 0xFFFF, r & 0xFFFF, wd, 0x1E);
    }
}

static void ent_option(u32 xo, u32 r, u32 wd, u32 sel, u32 v, u32 e, u32 rec)
{
    option(xo, r, wd, sel, v, (const char *)W32(e), SB8(e + 0x194), SB8(e + 0x195), rec);
}

void trigger_menu_draw(void)
{
    u32 x = H16(LAY + 0xF4), y = H16(LAY + 0xF6), w = H16(LAY + 0xF8), h = H16(LAY + 0xFA);
    u32 r;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);

    r = y + 0x19;
    label_row(x, r, 0x4008ABB8u, ENT(37));
    r += 0x37;
    ent_option(x + 0x14, r, 0x86, SYS + 0x27A, 0, ENT(41), 0x5B6);
    ent_option(x + 0x9E, r, 0x86, SYS + 0x27A, 1, ENT(42), 0x5C0);
    ent_option(x + 0x128, r, 0x86, SYS + 0x27A, 2, ENT(43), 0x5CA);

    r += 0x41;
    label_row(x, r, 0x4008AD28u, ENT(38));
    r += 0x37;
    ent_option(x + 0x14, r, 0xCB, SYS + 0x27B, 0, ENT(44), 0x5D4);
    ent_option(x + 0xE3, r, 0xCB, SYS + 0x27B, 1, ENT(45), 0x5DE);

    r += 0x41;
    label_row(x, r, 0x4008AE98u, ENT(39));
    r += 0x37;
    option(x + 0x14, r, 0xCB, SYS + 0x27C, 0, "CH1", 0, 0, 0x5E8);
    option(x + 0xE3, r, 0xCB, SYS + 0x27C, 1, "CH2", 0, 0, 0x5F2);

    r += 0x41;
    label_row(x, r, 0x4008B008u, ENT(40));
    r += 0x37;
    ent_option(x + 0x14, r, 0x63, SYS + 0x284, 0, ENT(140), 0x5FC);
    option(x + 0x7B, r, 0x64, SYS + 0x284, 1, "L1", 0, 0, 0x606);
    option(x + 0xE3, r, 0x64, SYS + 0x284, 2, "L2", 0, 0, 0x610);
    option(x + 0x14B, r, 0x63, SYS + 0x284, 3, "L3", 0, 0, 0x61A);
}
""")

wr(0x40023EA4, 3464, "plot_markers_draw",
   "Draws the markers at the edges of the plot (plot box SYS+4 x, +6 y, +8 width, +0xA height). Channel offset tags on the left: '1' in yellow 0xFFFF00 at offset SYS+0x60 "
   "when CH1 is on (SYS+0x3E), '2' in cyan 0x00FFFF at SYS+0x120 when CH2 is on (SYS+0x102); the tag sits at y + height - offset - 1, kept 8 px inside the plot. The trigger "
   "level tag 'T' (colour 0x8800FF, right edge x 0x3A3 .. 0x3B5) sits at y + height - half SYS+0x27E when SYS+0x275 and SYS+0x274 are set. The trigger position tag 'H' "
   "(green 0xFF00, top edge) sits at x + int64 SYS+0x570 / int64 SYS+0x590 + width / 2; past the left or right edge it becomes a sideways arrow. In the split view "
   "(SYS+0x2AA) the plot is cut at height / 3: the upper part gets small tags (font 0x42E64F58) with the levels scaled by 1/3, the lower part big tags with the levels "
   "scaled by 2/3, and the lower 'H' uses SYS+0x578 / SYS+0x5A0 (zoom).",
   """
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define SH16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define FONT 0x42E658D8u             /* normal font */
#define FONT_S 0x42E64F58u           /* small font */

void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void text_draw(u32 x, u32 y, const char *s);
void func_4002ecc4_span(u32 y, u32 x0, u32 x1);
void vline_draw(u32 x, u32 y0, u32 y1);
long long __aeabi_ldivmod(long long n, long long d);

static int clamp(int a, int lo, int hi)
{
    if (a < lo)
        return lo;
    return hi > a ? a : hi;
}

/* left tag with a 15 px arrow: box at x 1, text at x xt */
static void big_tag(int p, u32 col, u32 xt, const char *s)
{
    int i;

    draw_colour_set(col);
    rect_fill(1, (p - 7) & 0xFFFF, 0xB, 0xF);
    word_42e68c74_set(FONT);
    draw_colour_set(0);
    text_draw(xt, (p - 6) & 0xFFFF, s);
    draw_colour_set(col);
    for (i = 0; i < 8; i++) {
        func_4002ecc4_span((p - i) & 0xFFFF, 0xC, (0x13 - i) & 0xFFFF);
        func_4002ecc4_span((p + i) & 0xFFFF, 0xC, (0x13 - i) & 0xFFFF);
    }
}

static void small_tag(int p, u32 col, u32 xt, const char *s)
{
    int i;

    draw_colour_set(col);
    rect_fill(1, (p - 6) & 0xFFFF, 6, 0xD);
    word_42e68c74_set(FONT_S);
    draw_colour_set(col);
    for (i = 0; i <= 6; i++) {
        func_4002ecc4_span((p - i) & 0xFFFF, 7, (0xE - i) & 0xFFFF);
        func_4002ecc4_span((p + i) & 0xFFFF, 7, (0xE - i) & 0xFFFF);
    }
    draw_colour_set(0);
    text_draw(xt, (p - 6) & 0xFFFF, s);
}

static void channel_tag(u32 offa, u32 col, const char *s, u32 xn, u32 xu, u32 xl)
{
    int py = H16(SYS + 6), ph = H16(SYS + 0xA), t;

    if (!B8(SYS + 0x2AA)) {
        big_tag(clamp(ph + py - (int)W32(offa) - 1, py + 8, ph + py - 8), col, xn, s);
        return;
    }
    small_tag(clamp((ph - (int)W32(offa)) / 3 + py, py + 8, py + ph / 3 - 8), col, xu, s);
    t = py + ph / 3;
    big_tag(clamp((int)(2u * (u32)(ph - (int)W32(offa))) / 3 + t + 1, t + 9, ph + py - 8), col, xl, s);
}

static void trig_big(int p)
{
    int i;

    draw_colour_set(0x8800FF);
    for (i = 0; i < 8; i++) {
        func_4002ecc4_span((p - i) & 0xFFFF, 0x3A3 + i, 0x3AA);
        func_4002ecc4_span((p + i) & 0xFFFF, 0x3A3 + i, 0x3AA);
    }
    rect_fill(0x3AB, (p - 7) & 0xFFFF, 0xB, 0xF);
    word_42e68c74_set(FONT);
    draw_colour_set(0);
    text_draw(0x3AD, (p - 6) & 0xFFFF, "T");
}

static void trig_small(int p)
{
    int i;

    draw_colour_set(0x8800FF);
    for (i = 0; i <= 6; i++) {
        func_4002ecc4_span((p - i) & 0xFFFF, 0x3A8 + i, 0x3AF);
        func_4002ecc4_span((p + i) & 0xFFFF, 0x3A8 + i, 0x3AF);
    }
    rect_fill(0x3B0, (p - 6) & 0xFFFF, 6, 0xD);
    word_42e68c74_set(FONT_S);
    draw_colour_set(0);
    text_draw(0x3AE, (p - 5) & 0xFFFF, "T");
}

/* 'H' arrow pointing right (position past the right edge), centred on row yc */
static void h_right(int px, int pw, int yc)
{
    int xr = pw + px - 0x14, x6 = xr + 0xB, i;

    draw_colour_set(0xFF00);
    rect_fill(xr & 0xFFFF, (yc - 7) & 0xFFFF, 0xB, 0xF);
    word_42e68c74_set(FONT);
    for (i = 0; i < 8; i++) {
        func_4002ecc4_span((yc - i) & 0xFFFF, x6 & 0xFFFF, (x6 + 7 - i) & 0xFFFF);
        func_4002ecc4_span((yc + i) & 0xFFFF, x6 & 0xFFFF, (x6 + 7 - i) & 0xFFFF);
    }
    word_42e68c74_set(FONT);
    draw_colour_set(0);
    text_draw((x6 - 8) & 0xFFFF, (yc - 6) & 0xFFFF, "H");
}

static void h_left(int yc)
{
    int i;

    draw_colour_set(0xFF00);
    for (i = 0; i < 8; i++) {
        func_4002ecc4_span((yc - i) & 0xFFFF, i + 1, 8);
        func_4002ecc4_span((yc + i) & 0xFFFF, i + 1, 8);
    }
    rect_fill(9, (yc - 7) & 0xFFFF, 0xB, 0xF);
    word_42e68c74_set(FONT);
    draw_colour_set(0);
    text_draw(0xA, (yc - 6) & 0xFFFF, "H");
}

/* 'H' arrow pointing down at x p, box from row yt */
static void h_down(int p, int yt)
{
    int y2 = yt + 0xB, i;

    draw_colour_set(0xFF00);
    rect_fill((p - 7) & 0xFFFF, yt & 0xFFFF, 0xF, 0xB);
    word_42e68c74_set(FONT);
    for (i = 0; i < 8; i++) {
        vline_draw((p - i) & 0xFFFF, y2 & 0xFFFF, (y2 + 7 - i) & 0xFFFF);
        vline_draw((p + i) & 0xFFFF, y2 & 0xFFFF, (y2 + 7 - i) & 0xFFFF);
    }
    draw_colour_set(0);
    text_draw((p - 3) & 0xFFFF, (y2 - 0xB) & 0xFFFF, "H");
}

static void h_down_small(int p, int yt)
{
    int y2 = yt + 8, i;

    draw_colour_set(0xFF00);
    rect_fill((p - 6) & 0xFFFF, yt & 0xFFFF, 0xD, 8);
    word_42e68c74_set(FONT_S);
    for (i = 0; i <= 6; i++) {
        vline_draw((p - i) & 0xFFFF, y2 & 0xFFFF, (y2 + 7 - i) & 0xFFFF);
        vline_draw((p + i) & 0xFFFF, y2 & 0xFFFF, (y2 + 7 - i) & 0xFFFF);
    }
    draw_colour_set(0);
    text_draw((p - 3) & 0xFFFF, (y2 - 8) & 0xFFFF, "H");
}

/* trigger position on screen: x + position / scale + width / 2, at least 0 */
static int h_pos(u32 pos, u32 scale)
{
    int px = H16(SYS + 4), pw = H16(SYS + 8);
    int p = px + (int)(u32)__aeabi_ldivmod(S64(pos), S64(scale));

    p = (int)((u32)p + (u32)__aeabi_ldivmod((long long)pw, 2));
    return p < 0 ? 0 : p;
}

void plot_markers_draw(void)
{
    int px, py, pw, ph, p, t;

    if (B8(SYS + 0x3E))
        channel_tag(SYS + 0x60, 0xFFFF00, "1", 7, 5, 7);
    if (B8(SYS + 0x102))
        channel_tag(SYS + 0x120, 0xFFFF, "2", 4, 3, 4);

    if (B8(SYS + 0x275) && B8(SYS + 0x274)) {
        py = H16(SYS + 6);
        ph = H16(SYS + 0xA);
        if (!B8(SYS + 0x2AA)) {
            trig_big(clamp(ph + py - SH16(SYS + 0x27E), py + 8, ph + py - 8));
        } else {
            trig_small(clamp((ph - SH16(SYS + 0x27E)) / 3 + py, py + 8, py + ph / 3 - 8));
            t = py + ph / 3;
            trig_big(clamp((int)(2u * (u32)(ph - SH16(SYS + 0x27E))) / 3 + t + 1, t + 9, ph + py - 8));
        }
    }

    if (!B8(SYS + 0x580))
        return;
    px = H16(SYS + 4);
    pw = H16(SYS + 8);
    p = h_pos(SYS + 0x570, SYS + 0x590);
    py = H16(SYS + 6);
    if (!B8(SYS + 0x2AA)) {
        if (p > pw - 8)
            h_right(px, pw, py + 8);
        else if (p < 8)
            h_left(py + 8);
        else
            h_down(p, py + 1);
        return;
    }
    if (p > pw - 8)
        h_right(px, pw, py + 8);
    else if (p < 8)
        h_left(py + 8);
    else
        h_down_small(p, py + 1);
    p = h_pos(SYS + 0x578, SYS + 0x5A0);
    t = H16(SYS + 6) + H16(SYS + 0xA) / 3;
    if (p > pw - 8)
        h_right(px, pw, t + 9);
    else if (p < 8)
        h_left(t + 9);
    else
        h_down(p, t + 2);
}
""")
