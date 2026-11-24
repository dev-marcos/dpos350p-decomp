/*
 * bode_cursors_draw -- stock 0x40037C3C (1436 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When byte 0x42E180C8+0xAC60 is set (cursors on), draws the two Bode cursors (x positions = the words +0xAC64 / +0xAC68 from the plot x): dashed 0xBBBBBB vertical lines (2 of every 4 pixels), a 17 x 14 tag with a downward triangle and the labels "C" "1" / "C" "2", then three text rows under the plot (y = plot y + height - 55, 17 apart): "C1 = ", "C2 = " and "DC = " (the difference) each followed by the frequency (0xBBBBBB), the gain in dB (0xFF8800, level_label_db) and the phase (0x0088FF, phase_deg_draw), from the doubles +0xAC70.. +0xACB0 computed by trace_cursor_values. The C1 and C2 rows show the sign, the difference row does not.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "state/bode_block.h"

#define BB 0x42E180C8u
#define U16(v) ((unsigned short)(v))
#define TW (*(volatile unsigned short *)0x42E68BE4u)

static void row(u32 y, const char *lab, u32 sz, double f, double g, double ph, u32 sg)
{
    u32 x = BODES(plot_x) + 0xA;
    draw_colour_set(0xBBBBBB);
    half_42e68be2_set(sz);
    text_draw(U16(x), U16(y), lab);
    half_42e68be2_set(2);
    x += TW;
    if (sz != 4)
        x -= 2;
    text_draw(U16(x), U16(y), "= ");
    half_42e68be2_set(2);
    x += TW;
    draw_colour_set(0xBBBBBB);
    frequency_draw(U16(x), U16(y), 0, f);
    x += frequency_draw(U16(x), U16(y), 1, f) + 0x14;
    draw_colour_set(0xFF8800);
    level_label_db(U16(x), U16(y), sg, g);
    x += TW + 0x3C;
    draw_colour_set(0x88FF);
    phase_deg_draw(U16(x), U16(y), sg, ph);
}

static void tag(u32 x, u32 top)
{
    u32 j, y = top + 0xB;
    draw_colour_set(0xBBBBBB);
    rect_fill(U16(x - 8), U16(top), 0x11, 0xE);
    for (j = 0; j < 9; j++) {
        vline_draw(U16(x - j), U16(y), U16(y + 0xD - j));
        vline_draw(U16(x + j), U16(y), U16(y + 0xD - j));
    }
}

void bode_cursors_draw(void)
{
    u32 x1, top, i, x, y;
    int c1, c2, n;
    if (*(volatile unsigned char *)(BB + 0xAC60) == 0)
        return;
    x1 = BODES(plot_x) + 1;
    top = BODES(plot_y) + 2;
    c1 = BODES(cursor1);
    c2 = BODES(cursor2);
    n = ((int)(BODES(plot_y) + BODES(plot_h)) - (int)top + 1) / 4;
    word_42e68c74_set(0x42E65418u);
    draw_colour_set(0xBBBBBB);
    if (n != 0) {
        x = (u32)c1 + x1 + 0xFFFF;
        for (i = 0; i < (u32)n; i++)
            vline_draw(U16(x), U16(top + 4 * i), U16(top + 4 * i + 2));
        x = (u32)c2 + x1 + 0xFFFF;
        for (i = 0; i < (u32)n; i++)
            vline_draw(U16(x), U16(top + 4 * i), U16(top + 4 * i + 2));
    }
    x = (u32)c1 + BODES(plot_x);
    tag(x, top);
    draw_colour_set(0);
    half_42e68be2_set(2);
    text_draw(U16(x - 3), U16(top), "C");
    text_draw(U16(x - 1), U16(top + 9), "1");
    x = (u32)c2 + BODES(plot_x);
    tag(x, top);
    half_42e68be2_set(1);
    draw_colour_set(0);
    text_draw(U16(x - 3), U16(top), "C");
    text_draw(U16(x - 2), U16(top + 9), "2");
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E658D8u);
    y = BODES(plot_h) + BODES(plot_y) - 0x37;
    row(y, "C1 ", 4, BODES(c1_freq), BODES(c1_gain), BODES(c1_phase), 1);
    row(y + 0x11, "C2 ", 3, BODES(c2_freq), BODES(c2_gain), BODES(c2_phase), 1);
    row(y + 0x22, "DC ", 3, BODES(dc_freq), BODES(dc_gain), BODES(dc_phase), 0);
}
