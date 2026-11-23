/*
 * trigger_option_buttons -- stock 0x40053940 (552 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Trigger options window, buttons 0x5B6 + 10i: SYS+0x27A = 0..2 (the second also closes window 0x11), SYS+0x27B = 0..1, SYS+0x27C = 0..1, SYS+0x284 = 0..3; each invalidates from window 3 (r1 as the stock leaves it). Byte SYS+0x284 is rewritten when none of its buttons fired.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "state/touch_buttons.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

void inv_left(u32 id) __asm__("ui_window_invalidate_from");

void trigger_option_buttons(void)
{
    u32 v;
    trigger_option_buttons_update();
    if (BREC(146, state) == 2) {
        BREC(146, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_invalidate_from(3, 0x5B6);
        SI(trigger_mode) = 0;
    } else if (BREC(147, state) == 2) {
        BREC(147, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_close_raw(0x11);
        inv_left(3);
        SI(trigger_mode) = 1;
    } else if (BREC(148, state) == 2) {
        BREC(148, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_invalidate_from(3, 0);
        SI(trigger_mode) = 2;
    } else if (BREC(149, state) == 2) {
        BREC(149, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_invalidate_from(3, 0x5D4);
        SI(trigger_edge) = 0;
    } else if (BREC(150, state) == 2) {
        BREC(150, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_invalidate_from(3, 0);
        SI(trigger_edge) = 1;
    } else if (BREC(151, state) == 2) {
        BREC(151, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_invalidate_from(3, 0x5E8);
        SI(trigger_channel) = 0;
    } else if (BREC(152, state) == 2) {
        BREC(152, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_invalidate_from(3, 0);
        SI(trigger_channel) = 1;
    } else {
        if (BREC(153, state) == 2) {
            BREC(153, state) = 0;
            BREC_BYTE0 = 0;
            ui_window_invalidate_from(3, 0);
            v = 0;
        } else {
            v = SI(f_284);
            if (BREC(154, state) == 2) {
                BREC(154, state) = 0;
                BREC_BYTE0 = 0;
                ui_window_invalidate_from(3, 0);
                v = 1;
            } else if (BREC(155, state) == 2) {
                BREC(155, state) = 0;
                BREC_BYTE0 = 0;
                ui_window_invalidate_from(3, 0);
                v = 2;
            } else if (BREC(156, state) == 2) {
                BREC(156, state) = 0;
                BREC_BYTE0 = 0;
                ui_window_invalidate_from(3, 0);
                v = 3;
            }
        }
        SI(f_284) = v;
    }
}
