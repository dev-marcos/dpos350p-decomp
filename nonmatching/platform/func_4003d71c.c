/*
 * ui_window_01_redraw -- stock 0x4003D71C (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 1: draw target = back buffer, then the window's draw function 0x40019DB8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_back(void);
void func_40019db8(void);

void ui_window_01_redraw(void)
{
    fb_target_back();
    func_40019db8();
}
