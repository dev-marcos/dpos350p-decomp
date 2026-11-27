/*
 * levels_markers_draw -- stock 0x400606A0 (1264 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When the markers are on (W32 0x42E68668+0x58), draws the two frequency markers of the level view at x offsets +0x5C / +0x60 in the plot box: a dashed vertical line (colour 0x88FFFFFF) and a flag labelled C1 / C2 (colour 0xBBBBBB); then the readout panel at the bottom left of the plot: rows 'C1 = ', 'C2 = ', 'DC = ' each with the frequency (s64 +0x68 / +0x70 / +0x78), the level in dBmV (+0x80 / +0x88 / +0x90) and the voltage (+0x98 / +0xA0 / +0xA8), 0x11 pixels apart.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "state/levels_block.h"
#include "shared/access.h"

#define BLK 0x42E68668u
#define ADV 0x42E68BE4u

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
    if (LEVELS(markers_on) == 0)
        return;
    x = LEVELS(plot_x) + 1;
    r4 = LEVELS(plot_y) + 2;
    m1 = W32(BLK + 0x5C);
    m2 = W32(BLK + 0x60);
    n = (u32)((int)(LEVELS(plot_h) + LEVELS(plot_y) - r4 + 1) / 4);
    font_set(0x42E65418u);
    draw_colour_set(0x88FFFFFFu);
    if (n) {
        for (i = 0; i < n; i++)
            vline_draw((0xFFFF + m1 + x) & 0xFFFF, (r4 + 4 * i) & 0xFFFF, (r4 + 4 * i + 2) & 0xFFFF);
        for (i = 0; i < n; i++)
            vline_draw((0xFFFF + m2 + x) & 0xFFFF, (r4 + 4 * i) & 0xFFFF, (r4 + 4 * i + 2) & 0xFFFF);
    }
    r6 = r4 + 0xB;
    r8 = r6 + 0xD;
    r7 = m1 + LEVELS(plot_x);
    draw_colour_set(0xBBBBBB);
    marker_flag(r7, r4, r6, r8);
    draw_colour_set(0);
    text_spacing_set(2);
    text_draw((r7 - 3) & 0xFFFF, (r6 - 0xB) & 0xFFFF, (const char *)IMG(40060b90));
    text_draw((r7 - 1) & 0xFFFF, (r6 - 2) & 0xFFFF, (const char *)IMG(40060b94));
    r7 = m2 + LEVELS(plot_x);
    draw_colour_set(0xBBBBBB);
    marker_flag(r7, r4, r6, r8);
    text_spacing_set(1);
    draw_colour_set(0);
    text_draw((r7 - 3) & 0xFFFF, (r6 - 0xB) & 0xFFFF, (const char *)IMG(40060b90));
    text_draw((r7 - 2) & 0xFFFF, (r6 - 2) & 0xFFFF, (const char *)IMG(40060b98));
    text_spacing_set(1);
    font_set(0x42E658D8u);
    draw_colour_set(0xBBBBBB);
    x = LEVELS(plot_x) + 0xA;
    y = LEVELS(plot_h) + LEVELS(plot_y) - 0x37;
    text_spacing_set(4);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)IMG(40060b9c));
    text_spacing_set(2);
    x += H16(ADV);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)IMG(40060ba0));
    text_spacing_set(2);
    x += H16(ADV);
    readout(x, y, 0x68, 0x80, 1, 0x98, 1);
    y += 0x11;
    x = LEVELS(plot_x) + 0xA;
    text_spacing_set(3);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)IMG(40060d64));
    text_spacing_set(2);
    x = x + H16(ADV) - 2;
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)IMG(40060ba0));
    text_spacing_set(2);
    x += H16(ADV);
    readout(x, y, 0x70, 0x88, 1, 0xA0, 1);
    x = LEVELS(plot_x) + 0xA;
    y += 0x11;
    text_spacing_set(3);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)IMG(40060dc4));
    text_spacing_set(2);
    x = x + H16(ADV) - 2;
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)IMG(40060ba0));
    text_spacing_set(2);
    x += H16(ADV);
    readout(x, y, 0x78, 0x90, 0, 0xA8, 0);
}
