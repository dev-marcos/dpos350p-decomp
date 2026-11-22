/*
 * func_4002d2d4_d0 -- stock 0x4002D71C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4002D2D4(a, b, c, 0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void number_text_width(u32 a, u32 b, u32 c, u32 d);

void func_4002d2d4_d0(u32 a, u32 b, u32 c)
{
    number_text_width(a, b, c, 0);
}
