/*
 * calibration_button_handler -- stock 0x40056974 (208 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0xC28 released: shows message 0x9C00 at the centre of window rectangle 0x252, runs calib_run_4_channels, shows message 0x9C00 + 0x1A0, waits 700 ms and closes window 0x2A.
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

void touch_button_update_0C28(void);
void calib_run_4_channels(void);

void calibration_button_handler(void)
{
    u32 cx, cy;
    touch_button_update_0C28();
    cx = (unsigned short)(WR16(0x252) + (WR16(0x256) >> 1));
    cy = (unsigned short)(WR16(0x254) + (WR16(0x258) >> 1));
    if (TB(0xC28) != 2)
        return;
    TB(0xC28) = 0;
    TB(0) = 0;
    message_box_draw((int)cx + TX8(0x9C00 + 0x194), (int)cy + TX8(0x9C00 + 0x195), TX32(0x9C00), 0);
    calib_run_4_channels();
    message_box_draw((int)cx + TX8(0x9C00 + 0x334), (int)cy + TX8(0x9C00 + 0x335), TX32(0x9C00 + 0x1A0), 0);
    delay_ms(700);
    ui_window_close(0x2A);
}
