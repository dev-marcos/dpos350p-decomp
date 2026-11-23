/*
 * trace_span_full -- stock 0x4005F8D8 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4005F778(tr, 0, SYS+0x2C0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"
#include "sys_info.h"

void trace_span_full(u32 tr)
{
    trace_scale_segment(tr, 0, SI(f_2c0));
}
