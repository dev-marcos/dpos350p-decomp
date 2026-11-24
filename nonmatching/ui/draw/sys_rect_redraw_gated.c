/*
 * sys_rect_redraw_gated -- stock 0x4002CC6C (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: enabled_trace_layers_clear, sys_rect_fill, then plot_or_xy_grid_draw.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"
#include "ui/screens.h"

void sys_rect_redraw_gated(void)
{
    if (g_touch->serviced)
        return;
    enabled_trace_layers_clear();
    sys_rect_fill();
    plot_or_xy_grid_draw();
}
