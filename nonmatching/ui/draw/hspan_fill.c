/*
 * hspan_fill -- stock 0x4002EBA8 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * One-pixel-high span on row y from x0 to x1 inclusive: rect_fill(x0, y, (u16)(x1 - x0 + 1), 1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void hspan_fill(u32 y, u32 x0, u32 x1)
{
    rect_fill(x0, y, (x1 - x0 + 1) & 0xFFFF, 1);
}
