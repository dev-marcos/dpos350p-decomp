/*
 * ui_window_05_draw -- stock 0x4001C348 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draw function of window 5: touch_button_set_rect(rectangle of window record 5 = u16 at 0x42E64520+0x4C..+0x52, button record 0x42E620EA), then 0x400346C0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"

void ui_window_05_draw(void)
{
    volatile unsigned short *wn = (volatile unsigned short *)0x42E64520u;
    touch_button_set_rect(wn[0x26], wn[0x27], wn[0x28], wn[0x29], (volatile unsigned char *)0x42E620EAu);
    ui_frame_chain();
}
