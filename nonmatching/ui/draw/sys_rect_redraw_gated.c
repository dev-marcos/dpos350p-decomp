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

#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */

void sys_rect_redraw_gated(void)
{
    if (TCH8(0x17))
        return;
    enabled_trace_layers_clear();
    sys_rect_fill();
    plot_or_xy_grid_draw();
}
