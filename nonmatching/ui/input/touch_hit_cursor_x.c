/*
 * touch_hit_cursor_x -- stock 0x40069384 (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Horizontal distance from the touch x to the two cursors SYS_INFO+0x808/+0x80C (plus the x origin SYS+4, as u16), limited by touch state +4; ignored (0xFFFF) above the top third of the plot when SYS+0x2AA is set. Results +0x48/+0x4C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "sys_info.h"

#define TP      ((volatile short *)0x42E68BB0u)            /* touch block: x0, y0, x1, y1 (s16) */
#define TS32(o) (*(volatile u32 *)(0x42E68998u + (o)))     /* touch-control state */
#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_hit_cursor_x(void)
{
    int x = TP[2];
    u32 o = SI(plot_x0);
    int a = (unsigned short)(SI(cursor_x1) + o);
    int c = (unsigned short)(SI(cursor_x2) + o);
    u32 d0 = x >= a ? (u32)(x - a) : (u32)(a - x);
    u32 d1 = x >= c ? (u32)(x - c) : (u32)(c - x);
    if (TS32(4) < d0) d0 = 0xFFFF;
    if (TS32(4) < d1) d1 = 0xFFFF;
    if (SI(zoom_on) && TP[3] < (int)(SI(plot_y0) + ((SI(plot_height) * 0xAAABu) >> 17)))
        d0 = d1 = 0xFFFF;
    TS32(0x4C) = d1;
    TS32(0x48) = d0;
}
