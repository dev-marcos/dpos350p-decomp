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

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))

void language_select_buttons(void)
{
    touch_group_0156();
    if (TB(0x156) == 2) {
        TB(0x156) = 0;
        TB(0) = 0;
        SI(menu_language) = 1;
    } else if (TB(0x160) == 2) {
        TB(0x160) = 0;
        TB(0) = 0;
        SI(menu_language) = 2;
    } else if (TB(0x16A) == 2) {
        TB(0x16A) = 0;
        TB(0) = 0;
        SI(menu_language) = 3;
    } else if (TB(0x174) == 2) {
        TB(0x174) = 0;
        SI(menu_language) = 4;
    } else {
        return;
    }
    ui_text_language_apply();
    settings_b_save();
    ui_windows_mark_dirty();
}
