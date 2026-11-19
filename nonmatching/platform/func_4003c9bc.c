/*
 * trace_process_window -- stock 0x4003C9BC (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Processing of trace tr over the capture window: 0x4005F910, 0x4003C0AC, 0x4003BB6C, trace_c5cc_window, 0x4005E838, 0x4005DF5C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void trace_span_window_veneer(u32 tr);
void scaled_pair_update(u32 tr);
void trace_view_bounds(u32 tr);
void trace_c5cc_window(u32 tr);
void trace_intensity_accumulate_window(u32 tr);
void trace_intensity_draw_window(u32 tr);

void trace_process_window(u32 tr)
{
    trace_span_window_veneer(tr);
    scaled_pair_update(tr);
    trace_view_bounds(tr);
    trace_c5cc_window(tr);
    trace_intensity_accumulate_window(tr);
    trace_intensity_draw_window(tr);
}
