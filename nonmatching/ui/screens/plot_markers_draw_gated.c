/*
 * plot_markers_draw_gated -- stock 0x40023E90 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, branches to 0x40023EA4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/screens.h"

void plot_markers_draw_gated(void)
{
    if (g_touch->serviced == 0)
        plot_markers_draw();
}
