/*
 * timebase_index_step_down -- stock 0x4003BDE8 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS+0x55C and SYS+0x5FC are both below SYS+0x2AC (16 bit) x SYS+0x554: SYS+0x588 -= d, or 0 when d (full 32 bits) is larger. Then timebase_index_clamp and the 16-bit flags SYS+0x86E / SYS+0x870 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void timebase_index_clamp(void);

void timebase_index_step_down(u32 d)
{
    u32 lim = SYS16(0x2AC) * SYS32(0x554);
    if (SYS32(0x55C) < lim && SYS32(0x5FC) < lim) {
        u32 c = SYS8(0x588);
        SYS8(0x588) = c >= d ? c - d : 0;
    }
    timebase_index_clamp();
    SYS16(0x86E) = 1;
    SYS16(0x870) = 1;
}
