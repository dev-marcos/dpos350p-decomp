/*
 * trace_pointers_2b4 -- stock 0x40040030 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Time-base step with SYS_INFO+0x2B4 (0x4003FFE4).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void trace_pointers_2b4(void)
{
    trace_buffer_pointers_setup(SI(f_2b4));
}
