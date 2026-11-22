/*
 * touch_drag_delta -- stock 0x40069CF8 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Drag distance from the touch block: dx = x1 - x0, dy = y1 - y0 (signed 16-bit fields), divided by 10 when SYS_INFO+0x671 is set; stored at 0x42E68998+0x30 / +0x34.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_drag_delta(void)
{
    volatile short *t = (volatile short *)0x42E68BB0u;
    int dx = t[2] - t[0], dy = t[3] - t[1];
    if (SYS8(0x671)) {
        dx /= 10;
        dy /= 10;
    }
    *(volatile int *)(0x42E68998u + 0x34) = dy;
    *(volatile int *)(0x42E68998u + 0x30) = dx;
}
