/*
 * func_4002d82c_flag1 -- stock 0x4002D6D4 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4002D82C(a, b, c, d, the low byte of the fifth (stack) argument, 1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4002d82c(u32 a, u32 b, u32 c, u32 d, u32 e, u32 g);

void func_4002d82c_flag1(u32 a, u32 b, u32 c, u32 d, u32 e)
{
    func_4002d82c(a, b, c, d, e & 0xFF, 1);
}
