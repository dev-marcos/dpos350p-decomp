/*
 * touch_hit_x_b -- stock 0x40069524 (172 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * As touch_hit_cursor_x for the marks at SYS_INFO+0x3DA534/+0x3DA538. Results +0x58/+0x5C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "sys_info.h"
#include "state/touch_ctl.h"

#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_hit_x_b(void)
{
    int x = g_touch->down_x;
    u32 o = SI(plot_x0);
    int a = (unsigned short)(SI(cursor_a) + o);
    int c = (unsigned short)(SI(cursor_b) + o);
    u32 d0 = x >= a ? (u32)(x - a) : (u32)(a - x);
    u32 d1 = x >= c ? (u32)(x - c) : (u32)(c - x);
    if (TCS(cursor_limit) < d0) d0 = 0xFFFF;
    if (TCS(cursor_limit) < d1) d1 = 0xFFFF;
    if (SI(zoom_on) && g_touch->down_y < (int)(SI(plot_y0) + ((SI(plot_height) * 0xAAABu) >> 17)))
        d0 = d1 = 0xFFFF;
    TCS(f_05c) = d1;
    TCS(f_058) = d0;
}
