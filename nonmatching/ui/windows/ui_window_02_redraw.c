/*
 * ui_window_02_redraw -- stock 0x4003D734 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 2: draw target = back buffer, then the window's draw function 0x4001A708.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "ui/windows.h"

void ui_window_02_redraw(void)
{
    fb_target_back();
    side_menu_column_draw();
}
