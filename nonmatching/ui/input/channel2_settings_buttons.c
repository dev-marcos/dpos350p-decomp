/*
 * channel2_settings_buttons -- stock 0x400531B0 (496 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel 2 settings window, the same layout 0x64 buttons and 0xC4 SYS bytes further: buttons 0x3C2..0x41C, SYS+0x102, +0x104, +0x103, +0x106, +0x10A.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "ui/input.h"
#include "ui/windows.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_windows_invalidate_1_5_raw(void) __asm__("ui_windows_invalidate_1_5");

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

/* b = first button (toggle), s = SYS offset of the channel enable byte */
static void channel_buttons(u32 b, u32 s)
{
    if (TB(b) == 2) {
        TB(b) = 0;
        SYS8(s) = SYS8(s) == 0;
        if (SYS8(0x652))
            SYS8(s) = 1;
        TB(0) = 0;
    } else if (TB(b + 0xA) == 2) {
        TB(b + 0xA) = 0;
        TB(0) = 0;
        SYS8(s + 2) = 0;
    } else if (TB(b + 0x14) == 2) {
        TB(b + 0x14) = 0;
        TB(0) = 0;
        SYS8(s + 2) = 1;
    } else if (TB(b + 0x1E) == 2) {
        TB(b + 0x1E) = 0;
        TB(0) = 0;
        SYS8(s + 2) = 2;
    } else if (TB(b + 0x28) == 2) {
        TB(b + 0x28) = 0;
        TB(0) = 0;
        SYS8(s + 1) = 0;
    } else if (TB(b + 0x32) == 2) {
        TB(b + 0x32) = 0;
        TB(0) = 0;
        SYS8(s + 1) = 1;
    } else if (TB(b + 0x3C) == 2) {
        TB(b + 0x3C) = 0;
        SYS8(s + 4) = SYS8(s + 4) == 0;
        TB(0) = 0;
    } else if (TB(b + 0x5A) == 2) {
        TB(b + 0x5A) = 0;
        TB(0) = 0;
        SYS8(s + 8) = 0;
    } else if (TB(b + 0x46) == 2) {
        TB(b + 0x46) = 0;
        TB(0) = 0;
        SYS8(s + 8) = 1;
    } else if (TB(b + 0x50) == 2) {
        TB(b + 0x50) = 0;
        TB(0) = 0;
        SYS8(s + 8) = 2;
    } else {
        return;
    }
    outputs_apply();
    ui_windows_invalidate_1_5_raw();
}

void channel2_settings_buttons(void)
{
    touch_group_03C2();
    channel_buttons(0x3C2, 0x102);
}
