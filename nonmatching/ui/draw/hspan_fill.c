/*
 * hspan_fill -- stock 0x4002EBA8 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * (a, b, c): 0x4002ECC4(b, a, (u16)(c - b + 1), 1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void hspan_fill(u32 a, u32 b, u32 c)
{
    rect_fill(b, a, (c - b + 1) & 0xFFFF, 1);
}
