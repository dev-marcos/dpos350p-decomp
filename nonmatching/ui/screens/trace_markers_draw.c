/*
 * trace_markers_draw -- stock 0x40024C60 (436 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The two channel markers: '2' in a white 11 x 15 box at the left of the trace area at height SYS+6 + SYS+0xA - word SYS+0x120 - 1 (kept between SYS+6 + 8 and SYS+6 + SYS+0xA - 8), with a white triangle; and '1' in a yellow 15 x 11 box at the top, at x = word SYS+0x60 + the left edge of the centred square + 1, with a triangle drawn by vertical lines. Font 0x42E658D8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"

#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

void trace_markers_draw(void)
{
    int x = (int)(H16(SYS + 4) + (H16(SYS + 8) >> 1)) - (int)(H16(SYS + 0xA) >> 1);
    int bot = (int)(H16(SYS + 6) + H16(SYS + 0xA));
    int lo = (int)H16(SYS + 6) + 8, y = bot - (int)W32(SYS + 0x120) - 1, i, k;
    if (y >= lo)
        lo = (bot - 8 > y) ? y : bot - 8;
    draw_colour_set(0xFFFF);
    rect_fill(U16(x), U16(lo - 7), 0xB, 0xF);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0);
    text_draw(U16(x + 3), U16(lo - 6), "2");
    draw_colour_set(0xFFFF);
    x += 0xB;
    for (i = 0; i < 8; i++) {
        func_4002ecc4_span(U16(lo - i), U16(x), U16(x + 7 - i));
        func_4002ecc4_span(U16(lo + i), U16(x), U16(x + 7 - i));
    }
    x = (int)W32(SYS + 0x60) + ((int)(H16(SYS + 4) + (H16(SYS + 8) >> 1)) - (int)(H16(SYS + 0xA) >> 1)) + 1;
    k = (int)H16(SYS + 6) + 1;
    draw_colour_set(0xFFFF00);
    rect_fill(U16(x - 7), U16(k), 0xF, 0xB);
    word_42e68c74_set(0x42E658D8u);
    k += 0xB;
    for (i = 0; i < 8; i++) {
        vline_draw(U16(x - i), U16(k), U16(k + 7 - i));
        vline_draw(U16(x + i), U16(k), U16(k + 7 - i));
    }
    draw_colour_set(0);
    text_draw(U16(x + 0xFFFF), U16(k - 0xB), "1");
}
