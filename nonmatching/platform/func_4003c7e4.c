/*
 * trace_process_full -- stock 0x4003C7E4 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Full processing of trace tr, run only when SYS+0x101 is set and, for a 0x4003BF78(tr) result of 1, SYS+0x2AA is set too: 0x4005F90C, 0x4003C0AC, 0x4003C158, 0x4003C314, 0x4003BB6C, trace_c5cc_full, 0x4005E12C, 0x4005DDD4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

u32 mode_not_0_or_1(u32 tr);
void trace_span_full_veneer(u32 tr);
void scaled_pair_update(u32 tr);
void trace_shift_samples(u32 tr);
void trace_zoom_segments(u32 tr);
void trace_view_bounds(u32 tr);
void trace_c5cc_full(u32 tr);
void func_4005e12c(u32 tr);
void func_4005ddd4(u32 tr);

void trace_process_full(u32 tr)
{
    u32 k = mode_not_0_or_1(tr);
    if (SYS8(0x101) == 0)
        return;
    if (k == 1 && SYS8(0x2AA) == 0)
        return;
    trace_span_full_veneer(tr);
    scaled_pair_update(tr);
    trace_shift_samples(tr);
    trace_zoom_segments(tr);
    trace_view_bounds(tr);
    trace_c5cc_full(tr);
    func_4005e12c(tr);
    func_4005ddd4(tr);
}
