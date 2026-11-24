/*
 * bode_grid_draw -- stock 0x40038374 (1032 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, draws the Bode plot frame from the block 0x42E180C8 (u16 +0 x, +2 y, +4 width, +6 height, +8 decade width, +0xA decades, +0xC row height, +0xE rows; doubles +0x10/+0x18 gain top/bottom, +0x20/+0x28 phase top/bottom, +0x8D08/+0x8D10 log10 of the start / end frequency): clears the area, draws a double 0x444444 outline, sets touch button 0x42E62B3A to the plot, 3-pixel lines at each decade, the 2..9 minor lines at x + decade * log10(k), the row lines, the gain labels on the left (level_label_db at x = 2) and the phase labels on the right (phase_deg_draw at x = 943), first and last moved by 7 pixels, and 4 frequency labels 10^(start + i * span / decades) under the plot (the first left aligned, the last right aligned, the others centred on their decade).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "bode.h"
#include "libm.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "state/bode_block.h"

#define U16(v) ((unsigned short)(v))
#define TW (*(volatile unsigned short *)0x42E68BE4u)

void bode_grid_draw(void)
{
    u32 i, j, x, y, w;
    double step, d, base;
    if (g_touch->serviced)
        return;
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0);
    rect_fill(0, U16(BODES(plot_y) - 4), 0x3FC, U16(BODES(plot_h) + 0x1C));
    draw_colour_set(0x444444);
    rect_outline_draw(BODES(plot_x), BODES(plot_y), BODES(plot_w), BODES(plot_h));
    rect_outline_draw(U16(BODES(plot_x) + 1), U16(BODES(plot_y) + 1), U16(BODES(plot_w) + 0xFFFE), U16(BODES(plot_h) + 0xFFFE));
    touch_button_set_rect(BODES(plot_x), BODES(plot_y), BODES(plot_w), BODES(plot_h), (volatile unsigned char *)0x42E62B3Au);
    x = BODES(plot_x);
    y = BODES(plot_y) + 1;
    for (i = 0; i < BODES(decades); i++) {
        rect_fill(U16(x), U16(y), 3, U16(BODES(plot_h) + 0xFFFE));
        x += BODES(decade_w);
    }
    x = BODES(plot_x);
    y = BODES(plot_y) + 1;
    for (i = 0; i < BODES(decades); i++) {
        base = (double)x;
        for (j = 2; j < 10; j++) {
            d = log10((double)j);
            vline_draw(U16((int)(base + (double)BODES(decade_w) * d)), U16(y), U16(BODES(plot_h) + y + 0xFFFE));
        }
        x += BODES(decade_w);
    }
    x = BODES(plot_x) + 1;
    y = BODES(plot_y);
    for (i = 1; i < BODES(rows); i++) {
        y += BODES(row_h);
        hspan_fill(U16(y), U16(x), U16(BODES(plot_w) + x + 0xFFFD));
    }
    draw_colour_set(0x888888);
    step = (BODES(gain_top) - BODES(gain_bottom)) / (double)BODES(rows);
    y = BODES(plot_y) - 7;
    for (i = 0; i <= BODES(rows); i++) {
        d = BODES(gain_top) - (double)i * step;
        level_label_db(2, i == 0 ? U16(y + 7) : i == BODES(rows) ? U16(y + 0xFFF9) : U16(y), 1, d);
        y += BODES(row_h);
    }
    draw_colour_set(0x888888);
    step = (BODES(phase_top) - BODES(phase_bottom)) / (double)BODES(rows);
    y = BODES(plot_y) - 7;
    for (i = 0; i <= BODES(rows); i++) {
        d = BODES(phase_top) - (double)i * step;
        phase_deg_draw(0x3AF, i == 0 ? U16(y + 7) : i == BODES(rows) ? U16(y + 0xFFF9) : U16(y), 1, d);
        y += BODES(row_h);
    }
    draw_colour_set(0x888888);
    step = (BODES(log_end) - BODES(log_start)) / (double)BODES(decades);
    y = BODES(plot_h) + BODES(plot_y) + 5;
    x = 0x3AF;
    for (i = 0; i < 4; i++) {
        d = pow(10.0, BODES(log_start) + (double)i * step);
        w = frequency_draw(U16(x), U16(y), 1, d);
        x = BODES(plot_x);
        if (i == 3)
            x = 3 * BODES(decade_w) + x - w - 2;
        else if (i != 0)
            x = i * BODES(decade_w) + x - (w >> 1);
        frequency_draw(U16(x), U16(y), 0, d);
    }
}
