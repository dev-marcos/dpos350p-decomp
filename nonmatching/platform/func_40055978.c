/*
 * system_calibration_button_handler -- stock 0x40055978 (348 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x188 released: message 0x9C00 at the centre of window rectangle 0x19C, system_calibration, restores the area and shows the message for its result (-2: 0x9C00 + 0x4E0, -1: + 0x340, 0: + 0x680, 1: + 0x1A0; none otherwise), waits 1 s, closes window 0x1D and invalidates from window 5.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
void message_box_draw(int x, int y, u32 text, int align);
void delay_ms(u32 ms);
#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))
#define WR16(o) (*(volatile unsigned short *)(0x42E64520u + (o)))
#define MSG(cx, cy, b) message_box_draw((int)(cx) + TX8((b) + 0x194), (int)(cy) + TX8((b) + 0x195), TX32(b), 0)

void touch_button_update_0188(void);
int system_calibration(void);
void rect_42e68c38_draw(void);
void ui_window_invalidate_from(u32 id);   /* r1 is what ui_window_close left, as in the stock code */

void system_calibration_button_handler(void)
{
    u32 cx, cy, t, p;
    int rc;
    touch_button_update_0188();
    cx = (unsigned short)(WR16(0x19C) + (WR16(0x1A0) >> 1));
    cy = (unsigned short)(WR16(0x19E) + (WR16(0x1A2) >> 1));
    if (TB(0x188) != 2)
        return;
    TB(0x188) = 0;
    TB(0) = 0;
    MSG(cx, cy, 0x9C00);
    rc = system_calibration();
    rect_42e68c38_draw();
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
    ui_window_close(0x1D);
    ui_window_invalidate_from(5);
}
