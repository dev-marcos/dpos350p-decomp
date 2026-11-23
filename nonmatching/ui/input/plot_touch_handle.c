/*
 * plot_touch_handle -- stock 0x400525D4 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * plot_area_button_update, then 0x400545F8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void plot_touch_handle(void)
{
    plot_area_button_update();
    plot_touch_drag();
}
