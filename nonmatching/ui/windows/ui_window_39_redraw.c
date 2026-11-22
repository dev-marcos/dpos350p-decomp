/*
 * ui_window_39_redraw -- stock 0x4003DE4C (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 39: draw target = back buffer, then the window's draw function 0x4002B43C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "ui/windows.h"

void ui_window_39_redraw(void)
{
    fb_target_back();
    levels_param_bar_draw();
}
