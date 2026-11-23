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

#define TP      ((volatile short *)0x42E68BB0u)            /* touch block: x0, y0, x1, y1 (s16) */
#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_hit_x_b(void)
{
    int x = TP[2];
    u32 o = SI(plot_x0);
    int a = (unsigned short)(SI(f_3da534) + o);
    int c = (unsigned short)(SI(f_3da538) + o);
    u32 d0 = x >= a ? (u32)(x - a) : (u32)(a - x);
    u32 d1 = x >= c ? (u32)(x - c) : (u32)(c - x);
    if (TCS(cursor_limit) < d0) d0 = 0xFFFF;
    if (TCS(cursor_limit) < d1) d1 = 0xFFFF;
    if (SI(zoom_on) && TP[3] < (int)(SI(plot_y0) + ((SI(plot_height) * 0xAAABu) >> 17)))
        d0 = d1 = 0xFFFF;
    TCS(f_05c) = d1;
    TCS(f_058) = d0;
}
