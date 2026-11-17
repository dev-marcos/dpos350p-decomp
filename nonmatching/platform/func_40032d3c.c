/*
 * key_repeat_update -- stock 0x40032D3C (192 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Two repeat counters (u16 SYS+0x86E, +0x870) with request flags SYS+0x872, +0x873: when a flag is set it is cleared and the counter goes up when less than word SYS+0x87C ticks passed since its time stamp (SYS+0x874, +0x878), otherwise down to a minimum of 1; the stamp is then the current tick.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TICK (*(volatile u32 *)0x42E68CD8u)

void key_repeat_update(void)
{
    int c = SYS16(0x86E), d;
    if (SYS8(0x872)) {
        SYS8(0x872) = 0;
        if (TICK - SYS32(0x874) < SYS32(0x87C))
            c++;
        else if (c >= 2)
            c--;
        SYS32(0x874) = TICK;
    }
    SYS16(0x86E) = c;
    if (SYS8(0x873)) {
        SYS8(0x873) = 0;
        d = SYS16(0x870);
        if (TICK - SYS32(0x878) < SYS32(0x87C))
            SYS16(0x870) = d + 1;
        else if (d >= 2)
            SYS16(0x870) = d - 1;
        SYS32(0x878) = TICK;
    }
}
