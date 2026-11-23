/*
 * key_repeat_update -- stock 0x40032D3C (192 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Two repeat counters (u16 SYS+0x86E, +0x870) with request flags SYS+0x872, +0x873: when a flag is set it is cleared and the counter goes up when less than word SYS+0x87C ticks passed since its time stamp (SYS+0x874, +0x878), otherwise down to a minimum of 1; the stamp is then the current tick.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "sys_info.h"

#define TICK (*(volatile u32 *)0x42E68CD8u)

void key_repeat_update(void)
{
    int c = SI(ch1_avg_limit), d;
    if (SI(ch1_avg_ready)) {
        SI(ch1_avg_ready) = 0;
        if (TICK - SI(f_874) < SI(f_87c))
            c++;
        else if (c >= 2)
            c--;
        SI(f_874) = TICK;
    }
    SI(ch1_avg_limit) = c;
    if (SI(ch2_avg_ready)) {
        SI(ch2_avg_ready) = 0;
        d = SI(ch2_avg_limit);
        if (TICK - SI(f_878) < SI(f_87c))
            SI(ch2_avg_limit) = d + 1;
        else if (d >= 2)
            SI(ch2_avg_limit) = d - 1;
        SI(f_878) = TICK;
    }
}
