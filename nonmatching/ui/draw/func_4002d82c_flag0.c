/*
 * func_4002d82c_flag0 -- stock 0x4002D6F8 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4002D82C(a, b, c, d, the low byte of the fifth (stack) argument, 0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void number_draw(u32 a, u32 b, u32 c, u32 d, u32 e, u32 g);

void func_4002d82c_flag0(u32 a, u32 b, u32 c, u32 d, u32 e)
{
    number_draw(a, b, c, d, e & 0xFF, 0);
}
