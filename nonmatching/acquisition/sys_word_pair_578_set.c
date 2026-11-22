/*
 * sys_word_pair_578_set -- stock 0x400424C4 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x578 = lo, SYS+0x57C = hi.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void sys_word_pair_578_set(u32 lo, u32 hi)
{
    SYS32(0x578) = lo;
    SYS32(0x57C) = hi;
}
