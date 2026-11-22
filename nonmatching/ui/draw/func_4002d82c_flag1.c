/*
 * func_4002d82c_flag1 -- stock 0x4002D6D4 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4002D82C(a, b, c, d, the low byte of the fifth (stack) argument, 1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void number_draw_raw(u32 a, u32 b, u32 c, u32 d, u32 e, u32 g) __asm__("number_draw");

void func_4002d82c_flag1(u32 a, u32 b, u32 c, u32 d, u32 e)
{
    number_draw_raw(a, b, c, d, e & 0xFF, 1);
}
