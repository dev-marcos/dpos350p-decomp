/*
 * buttons_05c_handler -- stock 0x40052280 (564 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Window of button group 0x5C: 0x5C toggles SYS+0x3C (turning it off also clears SYS+0x665 and, if that was set, restores the saved state); 0x66 calls 0x4004C7C4; 0x70 resets the word pair 0x570 or 0x578 (by SYS+0x2AA) and sets SYS+0x27E, +0x60, +0x120 to 250; 0x7A / 0x8E step the byte SYS+0x41 (or SYS+0x105 when SYS+0x664 is set) up to SYS+0x1BA - 1 / down to 0, then apply the front end and invalidate from window 3; 0x84 toggles SYS+0x664 and invalidates from window 2; 0x98 calls 0x4006382C; 0xA2 opens window 0x13.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "frontend.h"
#include "system.h"
#include "ui/input.h"
#include "ui/windows.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

static void level_apply(void)
{
    TB(0) = 0;
    frontend_apply_all();
    inv_left(3);
}

void buttons_05c_handler(void)
{
    touch_group_005C();
    if (TB(0x5C) == 2) {
        TB(0x5C) = 0;
        TB(0) = 0;
        if (SYS8(0x3C) == 0) {
            SYS8(0x3C) = 1;
            return;
        }
        SYS8(0x3C) = 0;
        if (SYS8(0x665)) {
            SYS8(0x665) = 0;
            state_restore_all();
        }
    } else if (TB(0x66) == 2) {
        TB(0x66) = 0;
        TB(0) = 0;
        auto_set();
    } else if (TB(0x70) == 2) {
        TB(0x70) = 0;
        TB(0) = 0;
        if (SYS8(0x2AA) == 0)
            sys_word_pair_570_set(0, 0);
        else
            sys_word_pair_578_set(0, 0);
        SYS16(0x27E) = 0xFA;
        SYS32(0x60) = 0xFA;
        SYS32(0x120) = 0xFA;
    } else if (TB(0x7A) == 2) {
        int top;
        TB(0x7A) = 0;
        top = (int)SYS8(0x1BA) - 1;
        if (SYS8(0x664) == 0) {
            if ((int)SYS8(0x41) < top)
                SYS8(0x41) = SYS8(0x41) + 1;
        } else if ((int)SYS8(0x105) < top) {
            SYS8(0x105) = SYS8(0x105) + 1;
        }
        level_apply();
    } else if (TB(0x84) == 2) {
        u32 v;
        TB(0x84) = 0;
        v = SYS8(0x664) == 0;
        SYS8(0x664) = v;
        ui_window_invalidate_from(2, v);
    } else if (TB(0x8E) == 2) {
        TB(0x8E) = 0;
        if (SYS8(0x664) == 0) {
            if (SYS8(0x41))
                SYS8(0x41) = SYS8(0x41) - 1;
        } else if (SYS8(0x105)) {
            SYS8(0x105) = SYS8(0x105) - 1;
        }
        level_apply();
    } else if (TB(0x98) == 2) {
        TB(0x98) = 0;
        TB(0) = 0;
        trigger_level_auto();
    } else if (TB(0xA2) == 2) {
        TB(0xA2) = 0;
        TB(0) = 0;
        ui_window_open(0x13);
    }
}
