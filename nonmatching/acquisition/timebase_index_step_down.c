/*
 * timebase_index_step_down -- stock 0x4003BDE8 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS+0x55C and SYS+0x5FC are both below SYS+0x2AC (16 bit) x SYS+0x554: SYS+0x588 -= d, or 0 when d (full 32 bits) is larger. Then timebase_index_clamp and the 16-bit flags SYS+0x86E / SYS+0x870 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void timebase_index_step_down(u32 d)
{
    u32 lim = SI(f_2ac) * SI(f_554);
    if (SI(f_55c) < lim && SI(sample_limit) < lim) {
        u32 c = SI(timebase_index);
        SI(timebase_index) = c >= d ? c - d : 0;
    }
    timebase_index_clamp();
    SI(ch1_avg_limit) = 1;
    SI(ch2_avg_limit) = 1;
}
