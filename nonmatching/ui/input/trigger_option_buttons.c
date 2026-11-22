/*
 * trigger_option_buttons -- stock 0x40053940 (552 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Trigger options window, buttons 0x5B6 + 10i: SYS+0x27A = 0..2 (the second also closes window 0x11), SYS+0x27B = 0..1, SYS+0x27C = 0..1, SYS+0x284 = 0..3; each invalidates from window 3 (r1 as the stock leaves it). Byte SYS+0x284 is rewritten when none of its buttons fired.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
void ui_window_invalidate_from(u32 id, u32 r1);
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

void touch_group_05B6(void);

void trigger_option_buttons(void)
{
    u32 v;
    touch_group_05B6();
    if (TB(0x5B6) == 2) {
        TB(0x5B6) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0x5B6);
        SYS8(0x27A) = 0;
    } else if (TB(0x5C0) == 2) {
        TB(0x5C0) = 0;
        TB(0) = 0;
        ui_window_close(0x11);
        inv_left(3);
        SYS8(0x27A) = 1;
    } else if (TB(0x5CA) == 2) {
        TB(0x5CA) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0);
        SYS8(0x27A) = 2;
    } else if (TB(0x5D4) == 2) {
        TB(0x5D4) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0x5D4);
        SYS8(0x27B) = 0;
    } else if (TB(0x5DE) == 2) {
        TB(0x5DE) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0);
        SYS8(0x27B) = 1;
    } else if (TB(0x5E8) == 2) {
        TB(0x5E8) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0x5E8);
        SYS8(0x27C) = 0;
    } else if (TB(0x5F2) == 2) {
        TB(0x5F2) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0);
        SYS8(0x27C) = 1;
    } else {
        if (TB(0x5FC) == 2) {
            TB(0x5FC) = 0;
            TB(0) = 0;
            ui_window_invalidate_from(3, 0);
            v = 0;
        } else {
            v = SYS8(0x284);
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
        SYS8(0x284) = v;
    }
}
