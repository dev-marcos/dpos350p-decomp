/*
 * levels_toolbar_buttons -- stock 0x40056404 (292 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0xB56 (restore SYS_INFO, reset and clear the screen, open windows 1 and 5, SYS+0x54E = 0, SYS+0x5F0 = 100000000), 0xB60 (open window 0x29), 0xB6A (save a screenshot), 0xB74 (toggle word +0x58 of block 0x42E68668), 0xB7E (measurement setup unless locked), 0xB88 (toggle byte +0 of that block).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "system.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "state/levels_block.h"
#include "state/touch_buttons.h"



void levels_toolbar_buttons(void)
{
    levels_toolbar_buttons_update();
    if (BREC(290, state) == 2) {
        BREC(290, state) = 0;
        BREC_BYTE0 = 0;
        sys_info_restore();
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
        SI(f_54e) = 0;
        SI(sample_clock_hz) = 100000000;
    } else if (BREC(291, state) == 2) {
        BREC(291, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_open(0x29);
    } else if (BREC(292, state) == 2) {
        BREC(292, state) = 0;
        BREC_BYTE0 = 0;
        list_a_save_new();
    } else if (BREC(293, state) == 2) {
        BREC(293, state) = 0;
        BREC_BYTE0 = 0;
        LEVELS(markers_on) = LEVELS(markers_on) == 0;
    } else if (BREC(294, state) == 2) {
        BREC(294, state) = 0;
        BREC_BYTE0 = 0;
        measure_setup_gated();
    } else if (BREC(295, state) == 2) {
        BREC(295, state) = 0;
        BREC_BYTE0 = 0;
        *(volatile unsigned char *)0x42E68668u = *(volatile unsigned char *)0x42E68668u == 0;
    }
}
