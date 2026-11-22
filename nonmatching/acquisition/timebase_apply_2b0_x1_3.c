/*
 * timebase_apply_2b0_x1_3 -- stock 0x40040048 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Time-base step with SYS_INFO+0x2B0 x 1.3 (double, truncated).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void timebase_apply_2b0_x1_3(void)
{
    trace_buffer_pointers_setup((u32)((double)SYS32(0x2B0) * 1.3));
}
