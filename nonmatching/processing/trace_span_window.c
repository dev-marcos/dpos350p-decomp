/*
 * trace_span_window -- stock 0x4005F8E8 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4005F778(tr, SYS+0x6FC, SYS+0x6F6 (16 bit)).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"
#include "sys_info.h"

void trace_span_window(u32 tr)
{
    trace_scale_segment(tr, SI(runtime_fine_progress), SI(runtime_step));
}
