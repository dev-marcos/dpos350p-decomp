/*
 * channel1_settings_buttons -- stock 0x40052FC4 (488 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel 1 settings window, buttons 0x35E..0x3B8: toggle the channel (SYS+0x3E; kept on while SYS+0x652 is set), SYS+0x40 = 0..2, SYS+0x3F = 0..1, toggle SYS+0x42, SYS+0x46 = 0..2 (buttons 0x3B8, 0x3A4, 0x3AE in that order); then outputs_apply and ui_windows_invalidate_1_5.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
void ui_window_invalidate_from(u32 id, u32 r1);
void inv_left(u32 id) __asm__("ui_window_invalidate_from");
void outputs_apply(void);
void ui_windows_invalidate_1_5(void);

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
    ui_windows_invalidate_1_5();
}

void touch_group_035E(void);

void channel1_settings_buttons(void)
{
    touch_group_035E();
    channel_buttons(0x35E, 0x3E);
}
