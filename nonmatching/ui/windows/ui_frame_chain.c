/*
 * ui_frame_chain -- stock 0x400346C0 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * outputs_apply, sys_rect_redraw_gated, acquisition_dispatcher, acquisition_rearm, traces_update, zoom_views_update, 0x40042520, plot_or_trace_markers_draw, 0x4003117C, 0x40030F8C, 0x40031794, then time_cursors_draw_gated. Most of them do nothing while touch byte +0x17 is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"
#include "system.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "ui/windows.h"

void ui_frame_chain(void)
{
    outputs_apply();
    sys_rect_redraw_gated();
    acquisition_dispatcher();
    acquisition_rearm();
    traces_update();
    zoom_views_update();
    split_window_marker_draw();
    plot_or_trace_markers_draw();
    manual_cursors_draw();
    measure_panel_draw();
    cursor_panel_draw();
    time_cursors_draw_gated();
}
