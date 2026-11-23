/*
 * plot_or_trace_markers_draw -- stock 0x40024E14 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x652 == 0: the gate of 0x40023EA4, else the gate of 0x40024C60.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/screens.h"
#include "sys_info.h"

void plot_or_trace_markers_draw(void)
{
    if (SI(f_652) == 0)
        plot_markers_draw_gated();
    else
        trace_markers_draw_gated();
}
