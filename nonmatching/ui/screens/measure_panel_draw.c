/*
 * measure_panel_draw -- stock 0x40030F8C (400 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The measurement panel at the bottom of the screen, unless the touch byte 0x42E68BB0+0x17 or the byte SYS+0x29 is set. The list SYS+0x880 holds up to 24 s16 items (0 ends it): a positive item is that measurement of channel 0, a negative one of channel 1. Item i is in column i % 6 and row i / 6 (rows go up from y = 0x206): a 155 x 24 box in 0x44333333 (the last item, unless in column 5, gets a 149-pixel box with a slanted right edge), then measure_item_draw at (0xA2 * column + 0x10, 0x209 - 24 * row).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"

#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

void measure_panel_draw(void)
{
    volatile short *list = (volatile short *)(SYS + 0x880);
    int last = 0x17, i, k;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    if (*(volatile unsigned char *)(SYS + 0x29))
        return;
    for (i = 0; i < 24; i++)
        if (list[i] == 0) {
            last = i - 1;
            break;
        }
    for (i = 0; i < 24; i++) {
        int v = list[i], ch = 0, row = i / 6, col = i - 6 * row;
        u32 x = 0x9B * col + 0xB, y = 0x206 - 24 * row;
        if (v == 0)
            return;
        if (v < 0) {
            ch = 1;
            v = -v;
        }
        draw_colour_set(0x44333333u);
        if (col != 5 && i == last) {
            u32 x1 = U16(x + 0x95), x2 = x1;
            rect_fill(U16(x), U16(y), 0x95, 0x18);
            for (k = 0; k < 24; k++, y++, x2++)
                func_4002ecc4_span(U16(y), x1, U16(x2));
        } else {
            rect_fill(U16(x), U16(y), 0x9B, 0x18);
        }
        measure_item_draw(U16(0xA2 * col + 0x10), U16(0x209 - 24 * row), (unsigned char)ch, (unsigned char)v);
    }
}
