# UI drawing of the level (spectrum) view and the numeric keypad: the graph frame with its grid,
# level and frequency labels and the two side colour bars; the C1 / C2 markers with the readout
# panel; the 4 x 4 keypad box.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define D64(a) (*(volatile double *)(a))
#define BLK 0x42E68668u
#define ADV 0x42E68BE4u

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void text_draw(u32 x, u32 y, const char *s);
"""

wr(0x40062448, 1480, "levels_graph_draw",
   "Unless touch byte 0x42E68BB0+0x17 is set, draws the frame of the level view (block 0x42E68668): clears the area of the box at 0x42E6473A, draws two outlines around the plot box +0xB0..+0xC0 "
   "and makes it a touch button (record 0x42E62BD0), the vertical grid lines every +0xC4 and the horizontal ones every +0xCC, the level labels top +0x148 minus i x step +0x140 (with 'Ref' at the first "
   "when a side panel is off), the frequency labels from +0x120 to +0x128 in +0xC8 steps; and, for each side panel enabled by byte +1 / +2, its colour bar (tables 0x42E56C30 / 0x42E583A0, "
   "scale 1000 / 512) with the same frequency labels under it.",
   COM + """
void rect_outline_draw(u32 x, u32 y, u32 w, u32 h);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void level_value_draw(u32 x, u32 y, u32 c, double v);
u32 frequency_draw(u32 x, u32 y, u32 measure, double d);

/* the frequency labels under a plot whose left edge is x0 and whose bottom is y */
static u32 freq_labels(u32 x8, u32 x0off, u32 y)
{
    u32 j = 0, w, t;
    double step, v;
    step = (double)(S64(BLK + 0x128) - S64(BLK + 0x120)) / (double)(int)W32(BLK + 0xC8);
    do {
        v = (double)S64(BLK + 0x120) + (double)j * step;
        w = frequency_draw(x8 & 0xFFFF, y & 0xFFFF, 1, v);
        x8 = W32(BLK + x0off);
        if (j != 0) {
            t = j * W32(BLK + 0xC4) + x8;
            if (j == W32(BLK + 0xC8))
                x8 = t - w - 2;
            else
                x8 = t - (w >> 1);
        }
        frequency_draw(x8 & 0xFFFF, y & 0xFFFF, 0, v);
        j++;
    } while (W32(BLK + 0xC8) >= j);
    return x8;
}

/* a side colour bar: n rows from y0, colour table tab scaled by top */
static u32 colour_bar(u32 x8, u32 noff, u32 yoff, u32 tab, double top)
{
    u32 j = 0, y;
    double k = top / (double)(int)W32(BLK + noff);
    if (W32(BLK + noff) != 0) {
        do {
            x8 = 10;
            y = j + W32(BLK + yoff);
            draw_colour_set(W32(tab + 4 * (u32)(top - (double)j * k)));
            j++;
            func_4002ecc4_span(y & 0xFFFF, 10, 0x28);
        } while (j < W32(BLK + noff));
    }
    return x8;
}

void levels_graph_draw(void)
{
    u32 i, x, y, x8, sb;
    double v;
    if (B8(0x42E68BB0u + 0x17))
        return;
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0);
    rect_fill(H16(0x42E6473Au), (H16(0x42E64740u) + H16(0x42E6473Cu)) & 0xFFFF, H16(0x42E6473Eu),
              (H16(0x42E6474Au) - H16(0x42E6473Cu) - H16(0x42E64740u)) & 0xFFFF);
    draw_colour_set(0x444444);
    rect_outline_draw((0xFFFF + W32(BLK + 0xB0)) & 0xFFFF, (0xFFFF + W32(BLK + 0xB4)) & 0xFFFF,
                      (W32(BLK + 0xB8) + 2) & 0xFFFF, (W32(BLK + 0xC0) + 2) & 0xFFFF);
    rect_outline_draw((0xFFFE + W32(BLK + 0xB0)) & 0xFFFF, (0xFFFE + W32(BLK + 0xB4)) & 0xFFFF,
                      (W32(BLK + 0xB8) + 4) & 0xFFFF, (W32(BLK + 0xC0) + 4) & 0xFFFF);
    touch_button_set_rect(W32(BLK + 0xB0) & 0xFFFF, W32(BLK + 0xB4) & 0xFFFF, W32(BLK + 0xB8) & 0xFFFF,
                          W32(BLK + 0xC0) & 0xFFFF, (volatile unsigned char *)0x42E62BD0u);
    x = W32(BLK + 0xB0) + 1;
    y = W32(BLK + 0xB4);
    i = 0;
    if (W32(BLK + 0xC8) - 1 != 0) {
        do {
            i++;
            x += W32(BLK + 0xC4);
            rect_fill(x & 0xFFFF, y & 0xFFFF, 1, W32(BLK + 0xC0) & 0xFFFF);
        } while (i < W32(BLK + 0xC8) - 1);
    }
    i = 1;
    x = W32(BLK + 0xB0);
    y = W32(BLK + 0xB4);
    if (W32(BLK + 0xD0) >= 2) {
        do {
            i++;
            y += W32(BLK + 0xCC);
            func_4002ecc4_span(y & 0xFFFF, x & 0xFFFF, (0xFFFF + W32(BLK + 0xB8) + x) & 0xFFFF);
        } while (i < W32(BLK + 0xD0));
    }
    x8 = 5;
    draw_colour_set(0x888888);
    i = 0;
    sb = W32(BLK + 0xB4) - 7;
    do {
        v = D64(BLK + 0x148) - (double)i * D64(BLK + 0x140);
        if (i == 0) {
            level_value_draw(5, (sb + 6) & 0xFFFF, 1, v);
            if (!B8(BLK + 2) || !B8(BLK + 1))
                text_draw(6, (sb + 0x18) & 0xFFFF, (const char *)0x40062684u);
        } else {
            level_value_draw(5, (i == W32(BLK + 0xD0) ? 0xFFFA + sb : sb) & 0xFFFF, 1, v);
        }
        i++;
        sb += W32(BLK + 0xCC);
    } while (W32(BLK + 0xD0) >= i);
    draw_colour_set(0x888888);
    x8 = freq_labels(x8, 0xB0, W32(BLK + 0xC0) + W32(BLK + 0xB4) + 5);
    if (B8(BLK + 1)) {
        x8 = colour_bar(x8, 0xE0, 0xD8, 0x42E56C30u, 1000.0);
        draw_colour_set(0x888888);
        x8 = freq_labels(x8, 0xD4, W32(BLK + 0xE0) + W32(BLK + 0xD8) + 5);
    }
    if (B8(BLK + 2)) {
        x8 = colour_bar(x8, 0xF0, 0xE8, 0x42E583A0u, 512.0);
        draw_colour_set(0x888888);
        freq_labels(x8, 0xE4, W32(BLK + 0xF0) + W32(BLK + 0xE8) + 5);
    }
}
""")

wr(0x400606A0, 1264, "levels_markers_draw",
   "When the markers are on (W32 0x42E68668+0x58), draws the two frequency markers of the level view at x offsets +0x5C / +0x60 in the plot box: a dashed vertical line (colour 0x88FFFFFF) and a "
   "flag labelled C1 / C2 (colour 0xBBBBBB); then the readout panel at the bottom left of the plot: rows 'C1 = ', 'C2 = ', 'DC = ' each with the frequency (s64 +0x68 / +0x70 / +0x78), the level in dBmV "
   "(+0x80 / +0x88 / +0x90) and the voltage (+0x98 / +0xA0 / +0xA8), 0x11 pixels apart.",
   COM + """
void vline_draw(u32 x, u32 y0, u32 y1);
u32 frequency_draw(u32 x, u32 y, u32 measure, double d);
void level_label_dbmv(u32 x, u32 y, u32 c, double v);
void voltage_uv_draw(u32 x, u32 y, u32 sg, double d);

/* the triangle-topped flag of a marker at x r7 */
static void marker_flag(u32 r7, u32 r4, u32 r6, u32 r8)
{
    u32 k;
    rect_fill((r7 - 8) & 0xFFFF, r4 & 0xFFFF, 0x11, 0xE);
    for (k = 0; k < 9; k++) {
        vline_draw((r7 - k) & 0xFFFF, r6 & 0xFFFF, (r8 - k) & 0xFFFF);
        vline_draw((r7 + k) & 0xFFFF, r6 & 0xFFFF, (r8 - k) & 0xFFFF);
    }
}

/* the frequency, level and voltage of one readout row; x is after the '= ' */
static void readout(u32 x, u32 y, u32 f, u32 l, u32 lc, u32 vlt, u32 vs)
{
    u32 w;
    frequency_draw(x & 0xFFFF, y & 0xFFFF, 0, (double)S64(BLK + f));
    w = frequency_draw(x & 0xFFFF, y & 0xFFFF, 1, (double)S64(BLK + f));
    x = w + x + 0x14;
    level_label_dbmv(x & 0xFFFF, y & 0xFFFF, lc, D64(BLK + l));
    voltage_uv_draw((H16(ADV) + x + 0x3C) & 0xFFFF, y & 0xFFFF, vs, D64(BLK + vlt));
}

void levels_markers_draw(void)
{
    u32 x, r4, m1, m2, n, i, r6, r8, r7, y;
    if (W32(BLK + 0x58) == 0)
        return;
    x = W32(BLK + 0xB0) + 1;
    r4 = W32(BLK + 0xB4) + 2;
    m1 = W32(BLK + 0x5C);
    m2 = W32(BLK + 0x60);
    n = (u32)((int)(W32(BLK + 0xC0) + W32(BLK + 0xB4) - r4 + 1) / 4);
    word_42e68c74_set(0x42E65418u);
    draw_colour_set(0x88FFFFFFu);
    if (n) {
        for (i = 0; i < n; i++)
            vline_draw((0xFFFF + m1 + x) & 0xFFFF, (r4 + 4 * i) & 0xFFFF, (r4 + 4 * i + 2) & 0xFFFF);
        for (i = 0; i < n; i++)
            vline_draw((0xFFFF + m2 + x) & 0xFFFF, (r4 + 4 * i) & 0xFFFF, (r4 + 4 * i + 2) & 0xFFFF);
    }
    r6 = r4 + 0xB;
    r8 = r6 + 0xD;
    r7 = m1 + W32(BLK + 0xB0);
    draw_colour_set(0xBBBBBB);
    marker_flag(r7, r4, r6, r8);
    draw_colour_set(0);
    half_42e68be2_set(2);
    text_draw((r7 - 3) & 0xFFFF, (r6 - 0xB) & 0xFFFF, (const char *)0x40060B90u);
    text_draw((r7 - 1) & 0xFFFF, (r6 - 2) & 0xFFFF, (const char *)0x40060B94u);
    r7 = m2 + W32(BLK + 0xB0);
    draw_colour_set(0xBBBBBB);
    marker_flag(r7, r4, r6, r8);
    half_42e68be2_set(1);
    draw_colour_set(0);
    text_draw((r7 - 3) & 0xFFFF, (r6 - 0xB) & 0xFFFF, (const char *)0x40060B90u);
    text_draw((r7 - 2) & 0xFFFF, (r6 - 2) & 0xFFFF, (const char *)0x40060B98u);
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0xBBBBBB);
    x = W32(BLK + 0xB0) + 0xA;
    y = W32(BLK + 0xC0) + W32(BLK + 0xB4) - 0x37;
    half_42e68be2_set(4);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)0x40060B9Cu);
    half_42e68be2_set(2);
    x += H16(ADV);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)0x40060BA0u);
    half_42e68be2_set(2);
    x += H16(ADV);
    readout(x, y, 0x68, 0x80, 1, 0x98, 1);
    y += 0x11;
    x = W32(BLK + 0xB0) + 0xA;
    half_42e68be2_set(3);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)0x40060D64u);
    half_42e68be2_set(2);
    x = x + H16(ADV) - 2;
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)0x40060BA0u);
    half_42e68be2_set(2);
    x += H16(ADV);
    readout(x, y, 0x70, 0x88, 1, 0xA0, 1);
    x = W32(BLK + 0xB0) + 0xA;
    y += 0x11;
    half_42e68be2_set(3);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)0x40060DC4u);
    half_42e68be2_set(2);
    x = x + H16(ADV) - 2;
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)0x40060BA0u);
    half_42e68be2_set(2);
    x += H16(ADV);
    readout(x, y, 0x78, 0x90, 0, 0xA8, 0);
}
""")

wr(0x4005FF88, 1376, "numpad_draw",
   "Draws the numeric keypad in the box at 0x42E646F4 (x, y, w, h u16): a dark frame with the title string 0x42E68A8B, then 4 rows x 4 keys of 0x5A x 0x3C, 0x5D x 0x3F apart: digits 7 8 9 / 4 5 6 / "
   "1 2 3 / . 0 and the function keys (colour 0x202020). Each key is a touch button (records 0x42E62A18..0x42E62AAE), highlighted in the colour W32 0x409223B4 when pressed; the keypad mode byte "
   "0x42E68A89 = 1..6 disables some keys (11, 12, 15) and picks the glyphs of keys 7, 11 and 15 and the '+-' of key 12.",
   COM + """
#define REC 0x42E61FF8u

void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void touch_button_disable(volatile unsigned char *b);
void glyph_alpha_draw(u32 x, u32 y, u32 s);
u32 func_4002d2d4_d0(u32 lo, u32 hi, u32 dec);
void func_4002d82c_flag0(u32 x, u32 y, u32 lo, u32 hi, u32 dec);
u32 text_width(const char *s);

void numpad_draw(void)
{
    static const unsigned char lab[16] = {7, 8, 9, 100, 4, 5, 6, 100, 1, 2, 3, 100, 100, 0, 100, 100};
    static const unsigned short off[16] = {0xA66, 0xA70, 0xA7A, 0xA98, 0xA48, 0xA52, 0xA5C, 0xAA2,
                                           0xA2A, 0xA34, 0xA3E, 0xAAC, 0xA84, 0xA20, 0xA8E, 0xAB6};
    u32 x0, y0, w, h, x, ry, row, col, k, mode, lb, tw, gx, gs, dis;
    volatile unsigned char *rec;
    const char *s;
    x0 = H16(0x42E646F4u);
    y0 = H16(0x42E646F6u);
    w = H16(0x42E646F8u);
    h = H16(0x42E646FAu);
    draw_colour_set(0);
    rect_fill(x0, y0, w, h);
    y0 += 1;
    x = x0 + 1;
    word_42e68c74_set(0x42E66BD8u);
    half_42e68be2_set(5);
    draw_colour_set(0x202020);
    rect_fill(x & 0xFFFF, y0 & 0xFFFF, (w - 2) & 0xFFFF, 0x3C);
    draw_colour_set(0xCCCCCC);
    text_shadow_draw((x + 0x1E) & 0xFFFF, (y0 + 0x13) & 0xFFFF, (const char *)0x42E68A8Bu, 1, 2, 0);
    ry = y0 + 0x3F;
    k = 0;
    for (row = 4; row; row--) {
        for (col = 0; col < 4; col++, k++, x += 0x5D) {
            lb = lab[k];
            rec = (volatile unsigned char *)(REC + off[k]);
            draw_colour_set(lb == 100 ? 0x202020 : 0x303030);
            rect_fill(x & 0xFFFF, ry & 0xFFFF, 0x5A, 0x3C);
            mode = B8(0x42E68A89u);
            if (mode == 2 || mode == 3)
                dis = k == 11 || k == 15;
            else if (mode == 1)
                dis = k == 12;
            else if (mode == 4)
                dis = k == 15;
            else if (mode == 5)
                dis = k == 11 || k == 12 || k == 15;
            else
                dis = 0;
            if (dis) {
                touch_button_disable(rec);
            } else {
                touch_button_set_rect(x & 0xFFFF, ry & 0xFFFF, 0x5A, 0x3C, rec);
                if (rec[1]) {
                    draw_colour_set(W32(0x409223B4u));
                    rect_fill(x & 0xFFFF, ry & 0xFFFF, 0x5A, 0x3C);
                }
            }
            draw_colour_set(0xCCCCCC);
            if (lb <= 9) {
                tw = func_4002d2d4_d0(lb, 0, 0);
                func_4002d82c_flag0((x + ((0x5A - tw) >> 1)) & 0xFFFF, (ry + 0x12) & 0xFFFF, lb, 0, 0);
                continue;
            }
            gs = 0;
            gx = 0;
            s = 0;
            if (k == 3) {
                glyph_alpha_draw((x + 0x20) & 0xFFFF, (ry + 0x15) & 0xFFFF, 0x40087B94u);
                continue;
            } else if (k == 14) {
                s = (const char *)0x400604E8u;
            } else if (k == 12) {
                mode = B8(0x42E68A89u);
                if (mode == 2 || mode == 3 || mode == 4)
                    s = (const char *)0x400604ECu;
            } else if (k == 7) {
                switch (B8(0x42E68A89u)) {
                case 1: gx = 0x21; gs = 0x40087F10u; break;
                case 2: gx = 0x21; gs = 0x40088094u; break;
                case 3: gx = 0x10; gs = 0x40084E30u; break;
                case 4: gx = 0x1D; gs = 0x400879A0u; break;
                case 5: gx = 0x1F; gs = 0x40087D5Cu; break;
                case 6: gx = 0x23; gs = 0x4008C2B8u; break;
                }
            } else if (k == 11) {
                mode = B8(0x42E68A89u);
                if (mode == 1) {
                    gx = 0x1A; gs = 0x40087548u;
                } else if (mode == 4) {
                    gx = 0x26; gs = 0x4008C60Cu;
                }
            } else if (k == 15) {
                if (B8(0x42E68A89u) == 1) {
                    gx = 0x18; gs = 0x400872B4u;
                }
            }
            if (s) {
                tw = text_width(s);
                text_draw((x + ((0x5A - tw) >> 1)) & 0xFFFF, (ry + 0x12) & 0xFFFF, s);
            } else if (gs) {
                glyph_alpha_draw((x + gx) & 0xFFFF, (ry + 0x16) & 0xFFFF, gs);
            }
        }
        ry += 0x3F;
        x = x0 + 1;
    }
    (void)h;
}
""")
