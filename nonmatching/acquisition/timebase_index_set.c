/*
 * timebase_index_set -- stock 0x4003BE50 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x588 = v, then 0x4003BCF0, then the halfwords SYS+0x86E and +0x870 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void timebase_index_set(u32 v)
{
    SI(timebase_index) = v;
    timebase_index_clamp();
    SI(ch1_avg_limit) = 1;
    SI(ch2_avg_limit) = 1;
}
