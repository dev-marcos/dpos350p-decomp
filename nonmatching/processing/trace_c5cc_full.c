/*
 * trace_c5cc_full -- stock 0x4003C790 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4003C5CC(tr, 0, SYS16+8): the whole record.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void trace_c5cc_full(u32 tr)
{
    trace_segments_gather(tr, 0, SI(plot_width));
}
