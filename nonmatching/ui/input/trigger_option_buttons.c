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

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

void trigger_option_buttons(void)
{
    u32 v;
    touch_group_05B6();
    if (TB(0x5B6) == 2) {
        TB(0x5B6) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0x5B6);
        SI(trigger_mode) = 0;
    } else if (TB(0x5C0) == 2) {
        TB(0x5C0) = 0;
        TB(0) = 0;
        ui_window_close_raw(0x11);
        inv_left(3);
        SI(trigger_mode) = 1;
    } else if (TB(0x5CA) == 2) {
        TB(0x5CA) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0);
        SI(trigger_mode) = 2;
    } else if (TB(0x5D4) == 2) {
        TB(0x5D4) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0x5D4);
        SI(trigger_edge) = 0;
    } else if (TB(0x5DE) == 2) {
        TB(0x5DE) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0);
        SI(trigger_edge) = 1;
    } else if (TB(0x5E8) == 2) {
        TB(0x5E8) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0x5E8);
        SI(trigger_channel) = 0;
    } else if (TB(0x5F2) == 2) {
        TB(0x5F2) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0);
        SI(trigger_channel) = 1;
    } else {
        if (TB(0x5FC) == 2) {
            TB(0x5FC) = 0;
            TB(0) = 0;
            ui_window_invalidate_from(3, 0);
            v = 0;
        } else {
            v = SI(f_284);
            if (TB(0x606) == 2) {
                TB(0x606) = 0;
                TB(0) = 0;
                ui_window_invalidate_from(3, 0);
                v = 1;
            } else if (TB(0x610) == 2) {
                TB(0x610) = 0;
                TB(0) = 0;
                ui_window_invalidate_from(3, 0);
                v = 2;
            } else if (TB(0x61A) == 2) {
                TB(0x61A) = 0;
                TB(0) = 0;
                ui_window_invalidate_from(3, 0);
                v = 3;
            }
        }
        SI(f_284) = v;
    }
}
