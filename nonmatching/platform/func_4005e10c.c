/*
 * layer_clear -- stock 0x4005E10C (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * fill_u16(0x400954A4 + n x 0x12BE70, 0, 0x95B50): clears layer n (1022 x 600 pixels of 16 bit).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fill_u16(u32 dst, u32 v, u32 n);

void layer_clear(u32 n)
{
    fill_u16(0x400954A4u + n * 0x12BE70u, 0, 0x95B50);
}
