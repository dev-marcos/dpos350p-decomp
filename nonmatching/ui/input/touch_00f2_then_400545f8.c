/*
 * touch_00f2_then_400545f8 -- stock 0x400525D4 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update_00F2, then 0x400545F8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_00f2_then_400545f8(void)
{
    touch_button_update_00F2();
    plot_touch_drag();
}
