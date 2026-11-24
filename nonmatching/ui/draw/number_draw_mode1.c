/*
 * number_draw_mode1 -- stock 0x4002D6D4 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * number_draw(x, y, v, dec, 1): v is passed as two words (r2 low, r3 high), dec is the low
 * byte of the fifth (stack) argument.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void number_draw_raw(u32 x, u32 y, u32 v_lo, u32 v_hi, u32 dec, u32 mode) __asm__("number_draw");

void number_draw_mode1(u32 x, u32 y, u32 v_lo, u32 v_hi, u32 dec)
{
    number_draw_raw(x, y, v_lo, v_hi, dec & 0xFF, 1);
}
