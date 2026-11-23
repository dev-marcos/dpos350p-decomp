/*
 * file_lists_open_buttons -- stock 0x40052F08 (172 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x340, 0x34A, 0x354: the first one released resets the window stack and opens window 0x14 (list A, list_a_reload), 0x18 (reload_chain_40035744) or 0x1A (reload_chain_400378e0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "system.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"


void file_lists_open_buttons(void)
{
    file_lists_open_buttons_update();
    if (BREC(83, state) == 2) {
        BREC(83, state) = 0;
        BREC_BYTE0 = 0;
        ui_windows_reset();
        ui_window_open(0x14);
        list_a_reload();
    } else if (BREC(84, state) == 2) {
        BREC(84, state) = 0;
        BREC_BYTE0 = 0;
        ui_windows_reset();
        ui_window_open(0x18);
        reload_chain_40035744();
    } else if (BREC(85, state) == 2) {
        BREC(85, state) = 0;
        BREC_BYTE0 = 0;
        ui_windows_reset();
        ui_window_open(0x1A);
        reload_chain_400378e0();
    }
}
