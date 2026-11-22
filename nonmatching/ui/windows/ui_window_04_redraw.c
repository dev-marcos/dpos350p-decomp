/*
 * ui_window_04_redraw -- stock 0x4003D77C (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 4: draw target = back buffer, then the window's draw function 0x4001C200.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_back(void);
void battery_icon_draw(void);

void ui_window_04_redraw(void)
{
    fb_target_back();
    battery_icon_draw();
}
