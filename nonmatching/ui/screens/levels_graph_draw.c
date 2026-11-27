/*
 * levels_graph_draw -- stock 0x40062448 (1480 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, draws the frame of the level view (block 0x42E68668): clears the area of the box at 0x42E6473A, draws two outlines around the plot box +0xB0..+0xC0 and makes it a touch button (record 0x42E62BD0), the vertical grid lines every +0xC4 and the horizontal ones every +0xCC, the level labels top +0x148 minus i x step +0x140 (with 'Ref' at the first when a side panel is off), the frequency labels from +0x120 to +0x128 in +0xC8 steps; and, for each side panel enabled by byte +1 / +2, its colour bar (tables 0x42E56C30 / 0x42E583A0, scale 1000 / 512) with the same frequency labels under it.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "state/levels_block.h"
#include "shared/access.h"

#define BLK 0x42E68668u

/* the frequency labels under a plot whose left edge is x0 and whose bottom is y */
static u32 freq_labels(u32 x8, u32 x0off, u32 y)
{
    u32 j = 0, w, t;
    double step, v;
    step = (double)(LEVELS(stop_freq) - LEVELS(start_freq)) / (double)(int)LEVELS(grid_x_div);
    do {
        v = (double)LEVELS(start_freq) + (double)j * step;
        w = frequency_draw(x8 & 0xFFFF, y & 0xFFFF, 1, v);
        x8 = W32(BLK + x0off);
        if (j != 0) {
            t = j * LEVELS(grid_x_step) + x8;
            if (j == LEVELS(grid_x_div))
                x8 = t - w - 2;
            else
                x8 = t - (w >> 1);
        }
        frequency_draw(x8 & 0xFFFF, y & 0xFFFF, 0, v);
        j++;
    } while (LEVELS(grid_x_div) >= j);
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
            hspan_fill(y & 0xFFFF, 10, 0x28);
        } while (j < W32(BLK + noff));
    }
    return x8;
}

void levels_graph_draw(void)
{
    u32 i, x, y, x8, sb;
    double v;
    if (g_touch->serviced)
        return;
    text_spacing_set(2);
    font_set(0x42E658D8u);
    draw_colour_set(0);
    rect_fill(H16(0x42E6473Au), (H16(0x42E64740u) + H16(0x42E6473Cu)) & 0xFFFF, H16(0x42E6473Eu),
              (H16(0x42E6474Au) - H16(0x42E6473Cu) - H16(0x42E64740u)) & 0xFFFF);
    draw_colour_set(0x444444);
    rect_outline_draw((0xFFFF + LEVELS(plot_x)) & 0xFFFF, (0xFFFF + LEVELS(plot_y)) & 0xFFFF,
                      (LEVELS(plot_w) + 2) & 0xFFFF, (LEVELS(plot_h) + 2) & 0xFFFF);
    rect_outline_draw((0xFFFE + LEVELS(plot_x)) & 0xFFFF, (0xFFFE + LEVELS(plot_y)) & 0xFFFF,
                      (LEVELS(plot_w) + 4) & 0xFFFF, (LEVELS(plot_h) + 4) & 0xFFFF);
    touch_button_set_rect(LEVELS(plot_x) & 0xFFFF, LEVELS(plot_y) & 0xFFFF, LEVELS(plot_w) & 0xFFFF,
                          LEVELS(plot_h) & 0xFFFF, (volatile unsigned char *)0x42E62BD0u);
    x = LEVELS(plot_x) + 1;
    y = LEVELS(plot_y);
    i = 0;
    if (LEVELS(grid_x_div) - 1 != 0) {
        do {
            i++;
            x += LEVELS(grid_x_step);
            rect_fill(x & 0xFFFF, y & 0xFFFF, 1, LEVELS(plot_h) & 0xFFFF);
        } while (i < LEVELS(grid_x_div) - 1);
    }
    i = 1;
    x = LEVELS(plot_x);
    y = LEVELS(plot_y);
    if (LEVELS(grid_y_div) >= 2) {
        do {
            i++;
            y += LEVELS(grid_y_step);
            hspan_fill(y & 0xFFFF, x & 0xFFFF, (0xFFFF + LEVELS(plot_w) + x) & 0xFFFF);
        } while (i < LEVELS(grid_y_div));
    }
    x8 = 5;
    draw_colour_set(0x888888);
    i = 0;
    sb = LEVELS(plot_y) - 7;
    do {
        v = LEVELS(reference) - (double)i * LEVELS(attenuation);
        if (i == 0) {
            level_value_draw(5, (sb + 6) & 0xFFFF, 1, v);
            if (!LEVELS(panel_b_on) || !LEVELS(panel_a_on))
                text_draw(6, (sb + 0x18) & 0xFFFF, (const char *)IMG(40062684));
        } else {
            level_value_draw(5, (i == LEVELS(grid_y_div) ? 0xFFFA + sb : sb) & 0xFFFF, 1, v);
        }
        i++;
        sb += LEVELS(grid_y_step);
    } while (LEVELS(grid_y_div) >= i);
    draw_colour_set(0x888888);
    x8 = freq_labels(x8, 0xB0, LEVELS(plot_h) + LEVELS(plot_y) + 5);
    if (LEVELS(panel_a_on)) {
        x8 = colour_bar(x8, 0xE0, 0xD8, 0x42E56C30u, 1000.0);
        draw_colour_set(0x888888);
        x8 = freq_labels(x8, 0xD4, LEVELS(panel_a_rows) + LEVELS(panel_a_y) + 5);
    }
    if (LEVELS(panel_b_on)) {
        x8 = colour_bar(x8, 0xF0, 0xE8, 0x42E583A0u, 512.0);
        draw_colour_set(0x888888);
        freq_labels(x8, 0xE4, LEVELS(panel_b_rows) + LEVELS(panel_b_y) + 5);
    }
}
