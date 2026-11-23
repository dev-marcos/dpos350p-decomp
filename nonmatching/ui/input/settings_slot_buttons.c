/*
 * settings_slot_buttons -- stock 0x40055798 (476 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Settings slot window: buttons 0x124 + 10i set the slot SYS+0x7C0 = i + 1 (1..5) and, by the window mode byte 0x42E64587 (1 save, 2 load, 3 save block F then load), run settings_slot_save / settings_slot_load; then the message 0xAAA0 at the centre of window rectangle 0x76 for 500 ms, closes window 8, applies outputs and the generator and marks the windows dirty.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/timer.h"
#include "generator.h"
#include "storage/settings.h"
#include "system.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "sys_info.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))
#define WR16(o) (*(volatile unsigned short *)(0x42E64520u + (o)))

void settings_slot_buttons(void)
{
    u32 cx, cy, i, m;
    touch_group_0124();
    cx = (unsigned short)(WR16(0x76) + (WR16(0x7A) >> 1));
    cy = (unsigned short)(WR16(0x78) + (WR16(0x7C) >> 1));
    for (i = 0; i < 5; i++)
        if (TB(0x124 + 10 * i) == 2)
            break;
    if (i == 5)
        return;
    TB(0x124 + 10 * i) = 0;
    TB(0) = 0;
    SI(f_7c0) = i + 1;
    m = *(volatile unsigned char *)(0x42E64520u + 0x67);
    if (m == 1) {
        settings_slot_save();
    } else if (m == 2) {
        settings_slot_load();
    } else if (m == 3) {
        settings_f_save();
        settings_slot_load();
    }
    message_box_draw((int)cx + TX8(0xAAA0 + 0x194), (int)cy + TX8(0xAAA0 + 0x195), TX32(0xAAA0), 0);
    delay_ms(500);
    rect_42e68c38_draw();
    ui_window_close_raw(8);
    outputs_apply();
    gen_update();
    ui_windows_mark_dirty();
}
