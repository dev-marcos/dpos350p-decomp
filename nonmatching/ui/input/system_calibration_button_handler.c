/*
 * system_calibration_button_handler -- stock 0x40055978 (348 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x188 released: message 0x9C00 at the centre of window rectangle 0x19C, system_calibration, restores the area and shows the message for its result (-2: 0x9C00 + 0x4E0, -1: + 0x340, 0: + 0x680, 1: + 0x1A0; none otherwise), waits 1 s, closes window 0x1D and invalidates from window 5.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/timer.h"
#include "system.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"
#include "state/ui_windows.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");
void ui_window_invalidate_from_raw(u32 id) __asm__("ui_window_invalidate_from");   /* r1 is what ui_window_close left, as in the stock code */

#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))
#define MSG(cx, cy, b) message_box_draw((int)(cx) + TX8((b) + 0x194), (int)(cy) + TX8((b) + 0x195), TX32(b), 0)

void system_calibration_button_handler(void)
{
    u32 cx, cy, t, p;
    int rc;
    system_calibration_button_update();
    cx = (unsigned short)(WREC(29, x) + (WREC(29, w) >> 1));
    cy = (unsigned short)(WREC(29, y) + (WREC(29, h) >> 1));
    if (BREC(39, state) != 2)
        return;
    BREC(39, state) = 0;
    BREC_BYTE0 = 0;
    MSG(cx, cy, 0x9C00);
    rc = system_calibration();
    message_box_erase();
    switch (rc) {
    case -2: t = 0x4E0, p = 0x674; break;
    case -1: t = 0x340, p = 0x4D4; break;
    case 0: t = 0x680, p = 0x814; break;
    case 1: t = 0x1A0, p = 0x334; break;
    default: t = 0; break;
    }
    if (t)
        message_box_draw((int)cx + TX8(0x9C00 + p), (int)cy + TX8(0x9C00 + p + 1), TX32(0x9C00 + t), 0);
    delay_ms(1000);
    ui_window_close_raw(0x1D);
    ui_window_invalidate_from_raw(5);
}
