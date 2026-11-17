/*
 * ui_window_19_redraw -- stock 0x4003DA00 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 19: draw target = back buffer, then the window's draw function 0x40023D04.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_back(void);
void window_frame_23d04_draw(void);

void ui_window_19_redraw(void)
{
    fb_target_back();
    window_frame_23d04_draw();
}
