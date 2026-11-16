/*
 * timebase_apply_2b0 -- stock 0x4004003C (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Time-base step with SYS_INFO+0x2B0 (0x4003FFE4).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void trace_buffer_pointers_setup(u32 v);                   /* 0x4003FFE4 */

void timebase_apply_2b0(void)
{
    trace_buffer_pointers_setup(SYS32(0x2B0));
}
