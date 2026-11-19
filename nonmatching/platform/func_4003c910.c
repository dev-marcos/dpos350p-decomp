/*
 * traces_process_single -- stock 0x4003C910 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Processing used when SYS+0x652 is set: 0x4005F90C(0/1), 0x4003C0AC(0), 0x4003C158(0/1), 0x4003C314(0/1), 0x4003BB6C(0), 0x4005EDC0(0), 0x4005DDD4(0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void trace_span_full_veneer(u32 tr);
void scaled_pair_update(u32 tr);
void trace_shift_samples(u32 tr);
void trace_zoom_segments(u32 tr);
void trace_view_bounds(u32 tr);
void trace_intensity_accumulate(u32 tr);
void trace_intensity_draw(u32 tr);

void traces_process_single(void)
{
    trace_span_full_veneer(0);
    trace_span_full_veneer(1);
    scaled_pair_update(0);
    trace_shift_samples(0);
    trace_shift_samples(1);
    trace_zoom_segments(0);
    trace_zoom_segments(1);
    trace_view_bounds(0);
    trace_intensity_accumulate(0);
    trace_intensity_draw(0);
}
