/*
 * timebase_index_step -- stock 0x4003BDB0 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x588 += d (byte), timebase_index_clamp, then the 16-bit flags SYS+0x86E and SYS+0x870 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void timebase_index_clamp(void);

void timebase_index_step(u32 d)
{
    SYS8(0x588) = SYS8(0x588) + d;
    timebase_index_clamp();
    SYS16(0x86E) = 1;
    SYS16(0x870) = 1;
}
