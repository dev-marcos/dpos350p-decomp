/*
 * trace_c5cc_full -- stock 0x4003C790 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4003C5CC(tr, 0, SYS16+8): the whole record.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void trace_segments_gather(u32 tr, u32 start, u32 n);

void trace_c5cc_full(u32 tr)
{
    trace_segments_gather(tr, 0, SYS16(8));
}
