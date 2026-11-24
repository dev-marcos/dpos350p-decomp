/*
 * touch_drag_delta_c -- stock 0x400691D4 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Drag distance (x1 - x0, y1 - y0) into touch state +0x30/+0x34, divided by 10 when byte 0x42E6866B is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "state/touch_ctl.h"

#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_drag_delta_c(void)
{
    int dx = g_touch->down_x - g_touch->x, dy = g_touch->down_y - g_touch->y;
    if (*(volatile unsigned char *)(0x42E68668u + 3)) {
        dx /= 10;
        dy /= 10;
    }
    TCS(drag_dy) = dy;
    TCS(drag_dx) = dx;
}
