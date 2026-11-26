/*
 * buttons_20a_handler -- stock 0x400527C0 (552 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Window of button group 0x20A. While the slider byte 0x42E62203 is set: touch byte = 1 and SYS+0x654 = 100 * (x - lo) / (hi - lo) (sdiv), x = s16 0x42E68BB0 clamped to the halfwords lo = 0x42E62204, hi = 0x42E62208. Else buttons: 0x214 / 0x21E / 0x228 toggle SYS+0x653 / +0x652 / +0x655; 0x17E only clears; 0x232 closes windows 0xA, 5, 2, opens 0x1C and 5 and calls sys_3c_enter; 0x23C backs up the settings and generator state, reinitialises, and opens windows 0x22..0x24; 0x246 backs up the settings, opens windows 0x26..0x28 and sets SYS+0x54E = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "generator.h"
#include "system.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "state/touch_buttons.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

void inv_left(u32 id) __asm__("ui_window_invalidate_from");

void buttons_20a_handler(void)
{
    touch_group_020A();
    if (BREC(52, touch_code)) {
        int x, lo, hi, d, n;
        BREC_BYTE0 = 1;
        x = g_touch->x;
        lo = BREC(52, x0);
        hi = BREC(52, x1);
        if (x < lo)
            x = lo;
        else if (hi < x)
            x = hi;
        n = (int)((u32)(x - lo) * 100u);
        d = hi - lo;
        SI(waveform_light) = d == 0 ? 0 : d == -1 ? (int)(0u - (u32)n) : n / d;
        return;
    }
    if (BREC(53, state) == 2) {
        BREC_BYTE0 = 0;
        BREC(53, state) = 0;
        SI(palette_alt) = SI(palette_alt) == 0;
    } else if (BREC(54, state) == 2) {
        BREC_BYTE0 = 0;
        BREC(54, state) = 0;
        SI(f_652) = SI(f_652) == 0;
    } else if (BREC(55, state) == 2) {
        BREC_BYTE0 = 0;
        BREC(55, state) = 0;
        SI(grid_on) = SI(grid_on) == 0;
    } else if (BREC(38, state) == 2) {
        BREC(38, state) = 0;
        BREC_BYTE0 = 0;
    } else if (BREC(56, state) == 2) {
        BREC_BYTE0 = 0;
        BREC(56, state) = 0;
        ui_window_close_raw(0xA);
        ui_window_close_raw(5);
        ui_window_close_raw(2);
        ui_window_open(0x1C);
        ui_window_open(5);
        sys_3c_enter();
    } else if (BREC(57, state) == 2) {
        BREC(57, state) = 0;
        BREC_BYTE0 = 0;
        sys_info_backup();
        gen_state_backup();
        acq_defaults_reset();
        generator_state_init();
        ui_windows_reset();
        fb_clear_current();
        ui_window_open(0x22);
        ui_window_open(0x23);
        ui_window_open(0x24);
    } else if (BREC(58, state) == 2) {
        BREC(58, state) = 0;
        BREC_BYTE0 = 0;
        sys_info_backup();
        acq_defaults_reset();
        ui_windows_reset();
        fb_clear_current();
        ui_window_open(0x26);
        ui_window_open(0x27);
        ui_window_open(0x28);
        SI(f_54e) = 1;
    }
}
