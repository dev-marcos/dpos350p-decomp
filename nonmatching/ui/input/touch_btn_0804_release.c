/*
 * touch_btn_0804_release -- stock 0x40054F08 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Updates button +0x804 (0x4002FB48); on its release (state 2) clears it and the table byte +0, then opens window 0x17.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"


void touch_btn_0804_release(void)
{
    touch_button_update_0804();
    if (BREC(205, state) != 2)
        return;
    BREC(205, state) = 0;
    BREC_BYTE0 = 0;
    ui_window_open(0x17);
}
