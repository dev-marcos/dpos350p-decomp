/*
 * ui_window_28_redraw -- stock 0x4003DBB8 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 28: draw target = back buffer, then the window's draw function 0x40028084.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/windows.h"

void ui_window_28_redraw(void)
{
    fb_target_back();
    ui_colour_buttons_draw();
}
