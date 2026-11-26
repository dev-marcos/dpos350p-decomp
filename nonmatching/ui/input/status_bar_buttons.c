/*
 * status_bar_buttons -- stock 0x400524B4 (288 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0xAC, 0xB6, 0xC0, 0xCA, 0xD4, 0xDE open windows 0xD..0x12; button 0xE8 toggles byte SYS+0x671 and invalidates from window 3 (r1 = the new value).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "state/touch_buttons.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))

void status_bar_buttons(void)
{
    u32 i, v;
    status_bar_buttons_update();
    for (i = 0; i < 6; i++) {
        if (TB(0xAC + 10 * i) == 2) {
            TB(0xAC + 10 * i) = 0;
            BREC_BYTE0 = 0;
            ui_window_open(0xD + i);
            return;
        }
    }
    if (BREC(23, state) != 2)
        return;
    BREC(23, state) = 0;
    BREC_BYTE0 = 0;
    v = SI(fine_move) == 0;
    SI(fine_move) = v;
    ui_window_invalidate_from(3, v);
}
