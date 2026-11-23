/*
 * main_menu_buttons -- stock 0x40052100 (384 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x2, 0xC, 0x16, 0x20 open windows 6, 0xA, 0xB, 0xC; 0x2A / 0x34 toggle SYS+0x802 / +0x803; 0x48 is only cleared; 0x3E saves a waveform (list_b_save_new); 0x52 resets the zoom and toggles SYS+0x2AA when the zoom count SYS+0x588 is above 12.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "storage/files.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "state/touch_buttons.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))

void main_menu_buttons(void)
{
    u32 i;
    touch_group_0002();
    for (i = 0; i < 4; i++) {
        if (TB(2 + 10 * i) == 2) {
            TB(2 + 10 * i) = 0;
            BREC_BYTE0 = 0;
            ui_window_open(i == 0 ? 6 : 9 + i);
            return;
        }
    }
    if (BREC(4, state) == 2) {
        BREC(4, state) = 0;
        BREC_BYTE0 = 0;
        SI(cursor_x_on) = SI(cursor_x_on) == 0;
    } else if (BREC(5, state) == 2) {
        BREC(5, state) = 0;
        BREC_BYTE0 = 0;
        SI(cursor_y_on) = SI(cursor_y_on) == 0;
    } else if (BREC(7, state) == 2) {
        BREC(7, state) = 0;
        BREC_BYTE0 = 0;
    } else if (BREC(6, state) == 2) {
        BREC(6, state) = 0;
        BREC_BYTE0 = 0;
        list_b_save_new();
    } else if (BREC(8, state) == 2) {
        BREC(8, state) = 0;
        BREC_BYTE0 = 0;
        if (SI(timebase_index) > 12) {
            zoom_reset();
            SI(zoom_on) = SI(zoom_on) == 0;
        }
    }
}
