/*
 * list_a_window_buttons -- stock 0x40054F50 (448 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * List A window: any of the buttons 0x818, 0x822, 0x82C, 0x836, 0x80E pressed (state 1) invalidates from window 0x16. Released: 0x818 closes windows 0x17 and 0x16, redraws the thumbnails and opens 0x14; 0x822 clears the marks, marks the current entry (SYS+0x5DC6 + s16 SYS+0xFA4), counts (0x400353A8) and opens 0x15; 0x82C / 0x836 move the current entry SYS+0xFA4 by -1 / +1 (page SYS+0xF9E = entry / page size SYS+0xF9A, sdiv) and reload it; 0x80E closes window 0x17. All but 0x818 then invalidate from window 0x16.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "ui/input.h"
#include "ui/windows.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

static int sdv(int a, int b)
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int)(0u - (u32)a);
    return a / b;
}

void list_a_window_buttons(void)
{
    u32 b822, b836;
    touch_group_080E();
    if (TB(0x818) == 1) {
        inv_left(0x16);
    } else if ((b822 = TB(0x822)) == 1) {
        ui_window_invalidate_from(0x16, 1);
    } else if (TB(0x82C) == 1) {
        ui_window_invalidate_from(0x16, b822);
    } else if ((b836 = TB(0x836)) == 1) {
        ui_window_invalidate_from(0x16, 1);
    } else if (TB(0x80E) == 1) {
        ui_window_invalidate_from(0x16, b836);
    }
    if (TB(0x818) == 2) {
        TB(0x818) = 0;
        TB(0) = 0;
        ui_window_close_raw(0x17);
        ui_window_close_raw(0x16);
        list_a_page_thumbs();
        ui_window_open(0x14);
        return;
    }
    if (TB(0x822) == 2) {
        TB(0x822) = 0;
        TB(0) = 0;
        list_a_marks_clear();
        SYS8(0x5DC6 + (short)SYS16(0xFA4)) = 1;
        list_count_nonzero_400353a8();
        ui_window_open(0x15);
    } else if (TB(0x82C) == 2 || TB(0x836) == 2) {
        int d;
        short v;
        if (TB(0x82C) == 2) {
            TB(0x82C) = 0;
            d = -1;
        } else {
            TB(0x836) = 0;
            d = 1;
        }
        TB(0) = 0;
        v = (short)((short)SYS16(0xFA4) + d);
        SYS16(0xFA4) = v;
        SYS16(0xF9E) = sdv(v, SYS16(0xF9A));
        list_cursor_clamp_40035248();
        list_a_current_load();
    } else if (TB(0x80E) == 2) {
        TB(0x80E) = 0;
        TB(0) = 0;
        ui_window_close_raw(0x17);
    } else {
        return;
    }
    inv_left(0x16);
}
