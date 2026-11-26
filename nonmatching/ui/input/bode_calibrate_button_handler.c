/*
 * bode_calibrate_button_handler -- stock 0x400562F4 (268 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0xB4C released: message 0x9C00 at the centre of window rectangle 0x20C, runs 0x40039EB8, then message 0xD680 when byte 0x42E22DE1 or 0x42E22DE2 is 0, else message 0x9C00 + 0x1A0; waits 1 s and closes window 0x25.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "bode.h"
#include "drivers/timer.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "state/bode_block.h"
#include "state/touch_buttons.h"
#include "state/ui_windows.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))
#define MSG(cx, cy, b) message_box_draw((int)(cx) + TX8((b) + 0x194), (int)(cy) + TX8((b) + 0x195), TX32(b), 0)

void bode_calibrate_button_handler(void)
{
    u32 cx, cy;
    bode_calibrate_button_update();
    cx = (unsigned short)(WREC(37, x) + (WREC(37, w) >> 1));
    cy = (unsigned short)(WREC(37, y) + (WREC(37, h) >> 1));
    if (BREC(289, state) != 2)
        return;
    BREC(289, state) = 0;
    BREC_BYTE0 = 0;
    MSG(cx, cy, 0x9C00);
    bode_calibrate();
    if (BODES(a_reached) == 0 || BODES(b_reached) == 0)
        MSG(cx, cy, 0xD680);
    else
        message_box_draw((int)cx + TX8(0x9C00 + 0x334), (int)cy + TX8(0x9C00 + 0x335), TX32(0x9C00 + 0x1A0), 0);
    delay_ms(1000);
    ui_window_close_raw(0x25);
}
