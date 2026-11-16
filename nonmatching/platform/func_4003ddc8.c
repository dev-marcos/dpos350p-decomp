/*
 * ui_window_35_redraw -- stock 0x4003DDC8 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 35: draw target = back buffer, then the window's draw function 0x4002A7F4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_back(void);
void func_4002a7f4(void);

void ui_window_35_redraw(void)
{
    fb_target_back();
    func_4002a7f4();
}
