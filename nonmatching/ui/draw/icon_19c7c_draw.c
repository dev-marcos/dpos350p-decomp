/*
 * icon_19c7c_draw -- stock 0x40019C7C (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws a small icon at (x, y) in colour word 0x409223EC: a 10 x 15 box at (x, y + 3) and a triangle of 8 spans pointing left, rows y + 10 +- k from x + 10 to x + 20 - k.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "shared/access.h"

void icon_19c7c_draw(u32 x, u32 y)
{
    int k;
    draw_colour_set(W32(0x409223ECu));
    rect_fill(x, (unsigned short)(y + 3), 10, 15);
    for (k = 0; k < 8; k++) {
        hspan_fill((unsigned short)(y + 10 + k), (unsigned short)(x + 10), (unsigned short)(x + 20 - k));
        hspan_fill((unsigned short)(y + 10 - k), (unsigned short)(x + 10), (unsigned short)(x + 20 - k));
    }
}
