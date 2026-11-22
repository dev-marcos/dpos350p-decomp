/*
 * ui_window_23_redraw -- stock 0x4003DAC0 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 23: draw target = back buffer, then the window's draw function 0x40025DB4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "ui/windows.h"

void ui_window_23_redraw(void)
{
    fb_target_back();
    screenshot_view_frame_draw();
}
