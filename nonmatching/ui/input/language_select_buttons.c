/*
 * language_select_buttons -- stock 0x40055AD4 (196 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x156, 0x160, 0x16A, 0x174 set the language byte SYS+0x7AC to 1..4 (the last one leaves the touch byte set), then apply the texts, save settings block B and mark the windows dirty.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "ui/input.h"
#include "ui/text.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "state/touch_buttons.h"


void language_select_buttons(void)
{
    language_select_buttons_update();
    if (BREC(34, state) == 2) {
        BREC(34, state) = 0;
        BREC_BYTE0 = 0;
        SI(menu_language) = 1;
    } else if (BREC(35, state) == 2) {
        BREC(35, state) = 0;
        BREC_BYTE0 = 0;
        SI(menu_language) = 2;
    } else if (BREC(36, state) == 2) {
        BREC(36, state) = 0;
        BREC_BYTE0 = 0;
        SI(menu_language) = 3;
    } else if (BREC(37, state) == 2) {
        BREC(37, state) = 0;
        SI(menu_language) = 4;
    } else {
        return;
    }
    ui_text_language_apply();
    settings_b_save();
    ui_windows_mark_dirty();
}
