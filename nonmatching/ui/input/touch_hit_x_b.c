/*
 * touch_hit_x_b -- stock 0x40069524 (172 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * As touch_hit_cursor_x for the marks at SYS_INFO+0x3DA534/+0x3DA538. Results +0x58/+0x5C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TP      ((volatile short *)0x42E68BB0u)            /* touch block: x0, y0, x1, y1 (s16) */
#define TS32(o) (*(volatile u32 *)(0x42E68998u + (o)))     /* touch-control state */
#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_hit_x_b(void)
{
    int x = TP[2];
    u32 o = SYS16(4);
    int a = (unsigned short)(SYS32(0x3DA534) + o);
    int c = (unsigned short)(SYS32(0x3DA538) + o);
    u32 d0 = x >= a ? (u32)(x - a) : (u32)(a - x);
    u32 d1 = x >= c ? (u32)(x - c) : (u32)(c - x);
    if (TS32(4) < d0) d0 = 0xFFFF;
    if (TS32(4) < d1) d1 = 0xFFFF;
    if (SYS8(0x2AA) && TP[3] < (int)(SYS16(6) + ((SYS16(0xA) * 0xAAABu) >> 17)))
        d0 = d1 = 0xFFFF;
    TS32(0x5C) = d1;
    TS32(0x58) = d0;
}
