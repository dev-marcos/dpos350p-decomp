/*
 * ui_window_38_redraw -- stock 0x4003DE34 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 38: draw target = back buffer, then the window's draw function 0x4002AE60.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_back(void);
void levels_toolbar_draw(void);

void ui_window_38_redraw(void)
{
    fb_target_back();
    levels_toolbar_draw();
}
