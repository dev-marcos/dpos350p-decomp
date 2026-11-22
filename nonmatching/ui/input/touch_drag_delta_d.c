/*
 * touch_drag_delta_d -- stock 0x40069324 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Drag distance (x1 - x0, y1 - y0) into touch state +0x30/+0x34, divided by 10 when byte 0x42E1810E is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TP      ((volatile short *)0x42E68BB0u)            /* touch block: x0, y0, x1, y1 (s16) */
#define TS32(o) (*(volatile u32 *)(0x42E68998u + (o)))     /* touch-control state */
#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_drag_delta_d(void)
{
    int dx = TP[2] - TP[0], dy = TP[3] - TP[1];
    if (*(volatile unsigned char *)(0x42E180C8u + 0x46)) {
        dx /= 10;
        dy /= 10;
    }
    TS32(0x34) = dy;
    TS32(0x30) = dx;
}
