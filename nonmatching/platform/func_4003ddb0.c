/*
 * ui_window_34_redraw -- stock 0x4003DDB0 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 34: draw target = back buffer, then the window's draw function 0x4002A184.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_back(void);
void bode_toolbar_draw(void);

void ui_window_34_redraw(void)
{
    fb_target_back();
    bode_toolbar_draw();
}
