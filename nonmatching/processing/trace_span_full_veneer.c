/*
 * trace_span_full_veneer -- stock 0x4005F90C (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Branch to trace_span_full.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

void trace_span_full_veneer(u32 tr)
{
    trace_span_full(tr);
}
