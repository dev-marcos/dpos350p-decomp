/*
 * rect_outline_draw -- stock 0x4002EC4C (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Outline of the w x h rectangle at (x, y): top and bottom rows and the left and right columns, all with rect_fill (the firmware falls through into rect_fill for the last one).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void rect_fill(u32 x, u32 y, u32 w, u32 h);

void rect_outline_draw(u32 x, u32 y, u32 w, u32 h)
{
    rect_fill(x, y, w, 1);
    rect_fill(x, (unsigned short)(h + y + 0xFFFF), w, 1);
    rect_fill(x, (unsigned short)(y + 1), 1, (unsigned short)(h - 2));
    rect_fill((unsigned short)(w + x + 0xFFFF), (unsigned short)(y + 1), 1, (unsigned short)(h - 2));
}
