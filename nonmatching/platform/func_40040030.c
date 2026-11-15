/*
 * timebase_apply_2b4 -- stock 0x40040030 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Time-base step with SYS_INFO+0x2B4 (0x4003FFE4).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4003ffe4(u32 v);                   /* fragment of the time-base switch (OPEN) */

void timebase_apply_2b4(void)
{
    func_4003ffe4(SYS32(0x2B4));
}
