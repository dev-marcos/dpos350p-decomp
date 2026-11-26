/*
 * view_position_set -- stock 0x400424C4 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x578 = lo, SYS+0x57C = hi.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void view_position_set(u32 lo, u32 hi)
{
    SYS32(0x578) = lo;
    SYS32(0x57C) = hi;
}
