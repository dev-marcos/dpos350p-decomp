/*
 * config_menu_buttons -- stock 0x40055720 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x106, 0x110, 0x11A: the first one released (state 2) is cleared with the touch byte and window 8 is opened.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"


void config_menu_buttons(void)
{
    config_menu_buttons_update();
    if (BREC(26, state) == 2) {
        BREC(26, state) = 0;
    } else if (BREC(27, state) == 2) {
        BREC(27, state) = 0;
    } else if (BREC(28, state) == 2) {
        BREC(28, state) = 0;
    } else {
        return;
    }
    BREC_BYTE0 = 0;
    ui_window_open(8);
}
