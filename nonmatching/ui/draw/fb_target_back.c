/*
 * fb_target_back -- stock 0x400342DC (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draw target (the pointer at 0x40095498) = the back buffer 0x41BFBF68.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void fb_target_back(void)
{
    *(volatile u32 *)0x40095498u = 0x41BFBF68u;
}
