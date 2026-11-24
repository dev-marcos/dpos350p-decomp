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

#define BB 0x42E180C8u
#define BH(o) (*(volatile unsigned short *)(BB + (o)))
#define BW(o) (*(volatile int *)(BB + (o)))
#define BD(o) (*(volatile double *)(BB + (o)))
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
    rect_fill(0, U16(BH(2) - 4), 0x3FC, U16(BH(6) + 0x1C));
    draw_colour_set(0x444444);
    rect_outline_draw(BH(0), BH(2), BH(4), BH(6));
    rect_outline_draw(U16(BH(0) + 1), U16(BH(2) + 1), U16(BH(4) + 0xFFFE), U16(BH(6) + 0xFFFE));
    touch_button_set_rect(BH(0), BH(2), BH(4), BH(6), (volatile unsigned char *)0x42E62B3Au);
    x = BH(0);
    y = BH(2) + 1;
    for (i = 0; i < BH(0xA); i++) {
        rect_fill(U16(x), U16(y), 3, U16(BH(6) + 0xFFFE));
        x += BH(8);
    }
    x = BH(0);
    y = BH(2) + 1;
    for (i = 0; i < BH(0xA); i++) {
        base = (double)x;
        for (j = 2; j < 10; j++) {
            d = log10((double)j);
            vline_draw(U16((int)(base + (double)BH(8) * d)), U16(y), U16(BH(6) + y + 0xFFFE));
        }
        x += BH(8);
    }
    x = BH(0) + 1;
    y = BH(2);
    for (i = 1; i < BH(0xE); i++) {
        y += BH(0xC);
        hspan_fill(U16(y), U16(x), U16(BH(4) + x + 0xFFFD));
    }
    draw_colour_set(0x888888);
    step = (BD(0x10) - BD(0x18)) / (double)BH(0xE);
    y = BH(2) - 7;
    for (i = 0; i <= BH(0xE); i++) {
        d = BD(0x10) - (double)i * step;
        level_label_db(2, i == 0 ? U16(y + 7) : i == BH(0xE) ? U16(y + 0xFFF9) : U16(y), 1, d);
        y += BH(0xC);
    }
    draw_colour_set(0x888888);
    step = (BD(0x20) - BD(0x28)) / (double)BH(0xE);
    y = BH(2) - 7;
    for (i = 0; i <= BH(0xE); i++) {
        d = BD(0x20) - (double)i * step;
        phase_deg_draw(0x3AF, i == 0 ? U16(y + 7) : i == BH(0xE) ? U16(y + 0xFFF9) : U16(y), 1, d);
        y += BH(0xC);
    }
    draw_colour_set(0x888888);
    step = (BD(0x8D10) - BD(0x8D08)) / (double)BH(0xA);
    y = BH(6) + BH(2) + 5;
    x = 0x3AF;
    for (i = 0; i < 4; i++) {
        d = pow(10.0, BD(0x8D08) + (double)i * step);
        w = frequency_draw(U16(x), U16(y), 1, d);
        x = BH(0);
        if (i == 3)
            x = 3 * BH(8) + x - w - 2;
        else if (i != 0)
            x = i * BH(8) + x - (w >> 1);
        frequency_draw(U16(x), U16(y), 0, d);
    }
}
