/*
 * fb_target_offscreen -- stock 0x400342EC (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draw target (the pointer at 0x40095498) = the offscreen buffer 0x4233CBBC, used to compose a window before an animated reveal.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_offscreen(void)
{
    *(volatile u32 *)0x40095498u = 0x4233CBBCu;
}
