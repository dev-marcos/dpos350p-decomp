/*
 * touch_hit_cursor_y -- stock 0x40069428 (252 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Vertical distance from the touch y to the cursors SYS_INFO+0x810/+0x814 (plus the y origin SYS+6, as u16); with SYS+0x2AA set the cursor is mapped as 2/3 of its value below the top third, and touches above that third are ignored. Limit touch state +4; results +0x50/+0x54.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "sys_info.h"

#define TP      ((volatile short *)0x42E68BB0u)            /* touch block: x0, y0, x1, y1 (s16) */
#define TS32(o) (*(volatile u32 *)(0x42E68998u + (o)))     /* touch-control state */
#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

static unsigned short cur_y(u32 v)
{
    if (SI(zoom_on))
        return (int)(v << 1) / 3 + (SI(plot_y0) + ((SI(plot_height) * 0xAAABu) >> 17));
    return v + SI(plot_y0);
}

void touch_hit_cursor_y(void)
{
    int y = TP[3];
    int a = cur_y(SI(cursor_y1));
    u32 d0 = y >= a ? (u32)(y - a) : (u32)(a - y);
    int c = cur_y(SI(cursor_y2));
    u32 d1 = y >= c ? (u32)(y - c) : (u32)(c - y);
    if (TS32(4) < d0) d0 = 0xFFFF;
    if (TS32(4) < d1) d1 = 0xFFFF;
    if (SI(zoom_on) && y < (int)(SI(plot_y0) + ((SI(plot_height) * 0xAAABu) >> 17)))
        d0 = d1 = 0xFFFF;
    TS32(0x54) = d1;
    TS32(0x50) = d0;
}
