/*
 * func_4002d2d4_d0 -- stock 0x4002D71C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4002D2D4(a, b, c, 0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void number_text_width_raw(u32 a, u32 b, u32 c, u32 d) __asm__("number_text_width");

void func_4002d2d4_d0(u32 a, u32 b, u32 c)
{
    number_text_width_raw(a, b, c, 0);
}
