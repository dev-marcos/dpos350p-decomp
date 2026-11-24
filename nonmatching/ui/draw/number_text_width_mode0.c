/*
 * number_text_width_mode0 -- stock 0x4002D71C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * number_text_width(v, dec, 0): v is passed as two words (r0 low, r1 high).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void number_text_width_raw(u32 v_lo, u32 v_hi, u32 dec, u32 mode) __asm__("number_text_width");

void number_text_width_mode0(u32 v_lo, u32 v_hi, u32 dec)
{
    number_text_width_raw(v_lo, v_hi, dec, 0);
}
