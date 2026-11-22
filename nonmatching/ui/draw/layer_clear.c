/*
 * layer_clear -- stock 0x4005E10C (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * fill_u16(0x400954A4 + n x 0x12BE70, 0, 0x95B50): clears layer n (1022 x 600 pixels of 16 bit).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "util.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void fill_u16_raw(u32 dst, u32 v, u32 n) __asm__("fill_u16");

void layer_clear(u32 n)
{
    fill_u16_raw(0x400954A4u + n * 0x12BE70u, 0, 0x95B50);
}
