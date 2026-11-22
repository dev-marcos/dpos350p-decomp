/*
 * draw_target_default -- stock 0x400342CC (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The drawing target pointer 0x40095498 = 0x41FB8FE8 (pixel_put writes through this pointer).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void draw_target_default(void)
{
    *(volatile u32 *)0x40095498u = 0x41FB8FE8u;
}
