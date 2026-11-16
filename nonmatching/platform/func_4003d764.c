/*
 * ui_window_05_redraw -- stock 0x4003D764 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 5: draw target = back buffer, then the window's draw function 0x4001C348.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_back(void);
void func_4001c348(void);

void ui_window_05_redraw(void)
{
    fb_target_back();
    func_4001c348();
}
