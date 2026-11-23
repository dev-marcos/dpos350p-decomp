/*
 * timebase_index_step -- stock 0x4003BDB0 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x588 += d (byte), timebase_index_clamp, then the 16-bit flags SYS+0x86E and SYS+0x870 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void timebase_index_step(u32 d)
{
    SI(timebase_index) = SI(timebase_index) + d;
    timebase_index_clamp();
    SI(ch1_avg_limit) = 1;
    SI(ch2_avg_limit) = 1;
}
