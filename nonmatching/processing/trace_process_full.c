/*
 * trace_process_full -- stock 0x4003C7E4 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Full processing of trace tr, run only when SYS+0x101 is set and, for a 0x4003BF78(tr) result of 1, SYS+0x2AA is set too: 0x4005F90C, 0x4003C0AC, 0x4003C158, 0x4003C314, 0x4003BB6C, trace_c5cc_full, 0x4005E12C, 0x4005DDD4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"
#include "ui/draw.h"
#include "sys_info.h"

void trace_process_full(u32 tr)
{
    u32 k = mode_not_0_or_1(tr);
    if (SI(ch1_runtime_state) == 0)
        return;
    if (k == 1 && SI(zoom_on) == 0)
        return;
    trace_span_full_veneer(tr);
    scaled_pair_update(tr);
    trace_shift_samples(tr);
    trace_zoom_segments(tr);
    trace_view_bounds(tr);
    trace_c5cc_full(tr);
    trace_intensity_accumulate_lines(tr);
    trace_intensity_draw(tr);
}
