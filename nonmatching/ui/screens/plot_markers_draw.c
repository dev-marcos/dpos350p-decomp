/*
 * plot_markers_draw -- stock 0x40023EA4 (3464 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the markers at the edges of the plot (plot box SYS+4 x, +6 y, +8 width, +0xA height). Channel offset tags on the left: '1' in yellow 0xFFFF00 at offset SYS+0x60 when CH1 is on (SYS+0x3E), '2' in cyan 0x00FFFF at SYS+0x120 when CH2 is on (SYS+0x102); the tag sits at y + height - offset - 1, kept 8 px inside the plot. The trigger level tag 'T' (colour 0x8800FF, right edge x 0x3A3 .. 0x3B5) sits at y + height - half SYS+0x27E when SYS+0x275 and SYS+0x274 are set. The trigger position tag 'H' (green 0xFF00, top edge) sits at x + int64 SYS+0x570 / int64 SYS+0x590 + width / 2; past the left or right edge it becomes a sideways arrow. In the split view (SYS+0x2AA) the plot is cut at height / 3: the upper part gets small tags (font 0x42E64F58) with the levels scaled by 1/3, the lower part big tags with the levels scaled by 2/3, and the lower 'H' uses SYS+0x578 / SYS+0x5A0 (zoom).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define SH16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define FONT 0x42E658D8u             /* normal font */
#define FONT_S 0x42E64F58u           /* small font */

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
    int py = SI(plot_y0), ph = SI(plot_height), t;

    if (!SI(zoom_on)) {
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
    int px = SI(plot_x0), pw = SI(plot_width);
    int p = px + (int)(u32)__aeabi_ldivmod(S64(pos), S64(scale));

    p = (int)((u32)p + (u32)__aeabi_ldivmod((long long)pw, 2));
    return p < 0 ? 0 : p;
}

void plot_markers_draw(void)
{
    int px, py, pw, ph, p, t;

    if (SI(ch1_enabled))
        channel_tag(SYS + 0x60, 0xFFFF00, "1", 7, 5, 7);
    if (SI(ch2_enabled))
        channel_tag(SYS + 0x120, 0xFFFF, "2", 4, 3, 4);

    if (SI(f_275) && SI(f_274)) {
        py = SI(plot_y0);
        ph = SI(plot_height);
        if (!SI(zoom_on)) {
            trig_big(clamp(ph + py - SH16(SYS + 0x27E), py + 8, ph + py - 8));
        } else {
            trig_small(clamp((ph - SH16(SYS + 0x27E)) / 3 + py, py + 8, py + ph / 3 - 8));
            t = py + ph / 3;
            trig_big(clamp((int)(2u * (u32)(ph - SH16(SYS + 0x27E))) / 3 + t + 1, t + 9, ph + py - 8));
        }
    }

    if (!SI(f_580))
        return;
    px = SI(plot_x0);
    pw = SI(plot_width);
    p = h_pos(SYS + 0x570, SYS + 0x590);
    py = SI(plot_y0);
    if (!SI(zoom_on)) {
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
    t = SI(plot_y0) + SI(plot_height) / 3;
    if (p > pw - 8)
        h_right(px, pw, t + 9);
    else if (p < 8)
        h_left(t + 9);
    else
        h_down(p, t + 2);
}
