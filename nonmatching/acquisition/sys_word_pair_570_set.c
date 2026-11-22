/*
 * sys_word_pair_570_set -- stock 0x4003BE98 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x570 = lo, SYS+0x574 = hi (a 64-bit value in two words).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void sys_word_pair_570_set(u32 lo, u32 hi)
{
    SYS32(0x570) = lo;
    SYS32(0x574) = hi;
}
