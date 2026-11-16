/*
 * trace_span_window_veneer -- stock 0x4005F910 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Branch to trace_span_window.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void trace_span_window(u32 tr);

void trace_span_window_veneer(u32 tr)
{
    trace_span_window(tr);
}
