/*
 * timebase_apply_2b0_x1_3 -- stock 0x40040048 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Time-base step with SYS_INFO+0x2B0 x 1.3 (double, truncated).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4003ffe4(u32 v);                   /* fragment of the time-base switch (OPEN) */

void timebase_apply_2b0_x1_3(void)
{
    func_4003ffe4((u32)((double)SYS32(0x2B0) * 1.3));
}
