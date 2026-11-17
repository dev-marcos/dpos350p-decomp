/*
 * ui_window_40_redraw -- stock 0x4003DE64 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 40: draw target = back buffer, then the window's draw function 0x40062EE8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_back(void);
void ui_window_40_draw(void);

void ui_window_40_redraw(void)
{
    fb_target_back();
    ui_window_40_draw();
}
