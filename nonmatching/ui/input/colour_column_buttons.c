/*
 * colour_column_buttons -- stock 0x40055634 (228 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x9F8 and 0xA02 (the first also runs list_c_save_new) leave mode 0x3C and swap windows 0x1C/5 for 2/5; buttons 0xA0C and 0xA16 set byte 0x4091F398 to 0 or 1 and invalidate from window 5.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "system.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");


#define FLAG (*(volatile unsigned char *)0x4091F398u)

void colour_column_buttons(void)
{
    colour_column_buttons_update();
    if (BREC(255, state) == 2) {
        BREC(255, state) = 0;
        BREC_BYTE0 = 0;
        list_c_save_new();
    } else if (BREC(256, state) == 2) {
        BREC(256, state) = 0;
        BREC_BYTE0 = 0;
    } else {
        u32 r1;                      /* the stock leaves r1 = flag address or flag value */
        if (BREC(257, state) == 2) {
            BREC(257, state) = 0;
            BREC_BYTE0 = 0;
            FLAG = 0;
            r1 = 0x4091F398u;
        } else if (BREC(258, state) == 2) {
            BREC(258, state) = 0;
            BREC_BYTE0 = 0;
            FLAG = 1;
            r1 = 1;
        } else {
            return;
        }
        ui_window_invalidate_from(5, r1);
        return;
    }
    sys_3c_leave();
    ui_window_close_raw(0x1C);
    ui_window_close_raw(5);
    ui_window_open(2);
    ui_window_open(5);
}
