/*
 * calibration_b4c_button_handler -- stock 0x400562F4 (268 bytes, ARM)
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

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))
#define WR16(o) (*(volatile unsigned short *)(0x42E64520u + (o)))
#define MSG(cx, cy, b) message_box_draw((int)(cx) + TX8((b) + 0x194), (int)(cy) + TX8((b) + 0x195), TX32(b), 0)

#define TT8(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))

void calibration_b4c_button_handler(void)
{
    u32 cx, cy;
    touch_button_update_0B4C();
    cx = (unsigned short)(WR16(0x20C) + (WR16(0x210) >> 1));
    cy = (unsigned short)(WR16(0x20E) + (WR16(0x212) >> 1));
    if (TB(0xB4C) != 2)
        return;
    TB(0xB4C) = 0;
    TB(0) = 0;
    MSG(cx, cy, 0x9C00);
    bode_calibrate();
    if (TT8(0xAD19) == 0 || TT8(0xAD1A) == 0)
        MSG(cx, cy, 0xD680);
    else
        message_box_draw((int)cx + TX8(0x9C00 + 0x334), (int)cy + TX8(0x9C00 + 0x335), TX32(0x9C00 + 0x1A0), 0);
    delay_ms(1000);
    ui_window_close_raw(0x25);
}
