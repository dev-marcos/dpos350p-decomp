/*
 * system_settings_buttons -- stock 0x400525EC (468 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * System window: buttons 0xFC, 0x17E open windows 7 and 0x1D; 0x192 resets the windows, opens 0x20 and enters USB mode; 0x1A6, 0x1B0, 0x1BA, 0x1C4, 0x1CE set the auto-off word SYS+0x658 to 0, 15, 30, 45, 60 (then auto_off_arm and settings_b_save); 0x1D8, 0x1E2, 0x1F6 open windows 9, 0x1E, 0x1F.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "power.h"
#include "storage/settings.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "usb/core.h"
#include "sys_info.h"
#include "state/touch_buttons.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

void system_settings_buttons(void)
{
    u32 w, i;
    touch_group_017E();
    if (BREC(25, state) == 2) {
        BREC(25, state) = 0;
        BREC_BYTE0 = 0;
        w = 7;
    } else if (BREC(38, state) == 2) {
        BREC(38, state) = 0;
        BREC_BYTE0 = 0;
        w = 0x1D;
    } else if (BREC(40, state) == 2) {
        BREC(40, state) = 0;
        BREC_BYTE0 = 0;
        ui_windows_reset();
        ui_window_open(0x20);
        usb_mode_enter();
        return;
    } else {
        for (i = 0; i < 5; i++) {
            if (TB(0x1A6 + 10 * i) == 2) {
                TB(0x1A6 + 10 * i) = 0;
                BREC_BYTE0 = 0;
                SI(auto_off_minutes) = 15 * i;
                auto_off_arm();
                settings_b_save();
                return;
            }
        }
        if (BREC(47, state) == 2) {
            BREC(47, state) = 0;
            BREC_BYTE0 = 0;
            w = 9;
        } else if (BREC(48, state) == 2) {
            BREC(48, state) = 0;
            BREC_BYTE0 = 0;
            w = 0x1E;
        } else if (BREC(50, state) == 2) {
            BREC(50, state) = 0;
            BREC_BYTE0 = 0;
            w = 0x1F;
        } else {
            return;
        }
    }
    ui_window_open(w);
}
