/*
 * calibration_button_handler -- stock 0x40056974 (208 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0xC28 released: shows message 0x9C00 at the centre of window rectangle 0x252, runs calib_run_4_channels, shows message 0x9C00 + 0x1A0, waits 700 ms and closes window 0x2A.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/timer.h"
#include "selftest.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"
#include "state/ui_windows.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))

void calibration_button_handler(void)
{
    u32 cx, cy;
    calibration_button_update();
    cx = (unsigned short)(WREC(42, x) + (WREC(42, w) >> 1));
    cy = (unsigned short)(WREC(42, y) + (WREC(42, h) >> 1));
    if (BREC(311, state) != 2)
        return;
    BREC(311, state) = 0;
    BREC_BYTE0 = 0;
    message_box_draw((int)cx + TX8(0x9C00 + 0x194), (int)cy + TX8(0x9C00 + 0x195), TX32(0x9C00), 0);
    calib_run_4_channels();
    message_box_draw((int)cx + TX8(0x9C00 + 0x334), (int)cy + TX8(0x9C00 + 0x335), TX32(0x9C00 + 0x1A0), 0);
    delay_ms(700);
    ui_window_close_raw(0x2A);
}
