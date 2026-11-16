/*
 * ui_window_36_redraw -- stock 0x4003DDE0 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 36: draw target = back buffer, then the window's draw function 0x40037AB8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fb_target_back(void);
void func_40037ab8(void);

void ui_window_36_redraw(void)
{
    fb_target_back();
    func_40037ab8();
}
