/*
 * levels_graph_draw -- stock 0x40062448 (1480 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, draws the frame of the level view (block 0x42E68668): clears the area of the box at 0x42E6473A, draws two outlines around the plot box +0xB0..+0xC0 and makes it a touch button (record 0x42E62BD0), the vertical grid lines every +0xC4 and the horizontal ones every +0xCC, the level labels top +0x148 minus i x step +0x140 (with 'Ref' at the first when a side panel is off), the frequency labels from +0x120 to +0x128 in +0xC8 steps; and, for each side panel enabled by byte +1 / +2, its colour bar (tables 0x42E56C30 / 0x42E583A0, scale 1000 / 512) with the same frequency labels under it.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

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
