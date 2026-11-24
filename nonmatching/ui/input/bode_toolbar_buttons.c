/*
 * bode_toolbar_buttons -- stock 0x40055F2C (340 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0xAC0 (restore SYS_INFO and the generator state, reset and clear the screen, open windows 1 and 5), 0xACA (open window 0x25), 0xAD4 (save a screenshot), 0xADE / 0xAF2 (toggle bytes 0x42E22D28 / 0x42E1810C), 0xAFC (toggle byte 0x42E1810D, copied to 0x42E1810F).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "storage/files.h"
#include "system.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/bode_block.h"
#include "state/touch_buttons.h"

void bode_toolbar_buttons(void)
{
    u32 v;
    bode_toolbar_buttons_update();
    if (BREC(275, state) == 2) {
        BREC(275, state) = 0;
        BREC_BYTE0 = 0;
        sys_info_restore();
        gen_state_restore();
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
    } else if (BREC(276, state) == 2) {
        BREC(276, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_open(0x25);
    } else if (BREC(277, state) == 2) {
        BREC(277, state) = 0;
        BREC_BYTE0 = 0;
        list_a_save_new();
    } else if (BREC(278, state) == 2) {
        BREC(278, state) = 0;
        BREC_BYTE0 = 0;
        BODES(cursors_on) = BODES(cursors_on) == 0;
    } else if (BREC(280, state) == 2) {
        BREC(280, state) = 0;
        BREC_BYTE0 = 0;
        BODES(loop) = BODES(loop) == 0;
    } else if (BREC(281, state) == 2) {
        BREC(281, state) = 0;
        BREC_BYTE0 = 0;
        v = BODES(running) == 0;
        BODES(running) = v;
        BODES(sweep_running) = v;
    }
}
