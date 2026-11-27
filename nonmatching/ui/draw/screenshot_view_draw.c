/*
 * screenshot_view_draw -- stock 0x40025D08 (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Registers the rectangle at 0x42E6465A as touch button 0x42E627FC, then copies the 1022 x 600 image at 0x42468A58 to the drawing target, rows in reverse order (bottom-up bitmap).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "util.h"

void screenshot_view_draw(void)
{
    volatile unsigned short *r = (volatile unsigned short *)(0x42E64520u + 0x13A);
    const unsigned short *s = (const unsigned short *)0x42468A58u;
    int i, o = 0;
    touch_button_set_rect(r[0], r[1], r[2], r[3], (volatile unsigned char *)0x42E627FCu);
    for (i = 600; i; i--) {
        copy_u16((unsigned short *)(*(volatile u32 *)IMG(40095498) + o + 0x12AEA4u), s, 0x3FE);
        s += 0x3FE;
        o -= 0x7FC;
    }
}
