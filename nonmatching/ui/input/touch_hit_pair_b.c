/*
 * touch_hit_pair_b -- stock 0x40069234 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Like touch_hit_pair_a for the marks at 0x42E180C8+0xAC64/+0xAC68 plus the u16 at 0x42E180C8 (32-bit sums, no truncation), limit +0x60, results +0x64/+0x68.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

#define TP      ((volatile short *)0x42E68BB0u)            /* touch block: x0, y0, x1, y1 (s16) */
#define TS32(o) (*(volatile u32 *)(0x42E68998u + (o)))     /* touch-control state */
#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_hit_pair_b(void)
{
    volatile unsigned char *b = (volatile unsigned char *)0x42E180C8u;
    int x = TP[2];
    u32 o = *(volatile unsigned short *)b;
    int a = (int)(*(volatile u32 *)(b + 0xAC64) + o);
    int c = (int)(*(volatile u32 *)(b + 0xAC68) + o);
    u32 d0 = x >= a ? (u32)(x - a) : (u32)(a - x);
    u32 d1 = x >= c ? (u32)(x - c) : (u32)(c - x);
    if (TS32(0x60) < d0) d0 = 0xFFFF;
    if (TS32(0x60) < d1) d1 = 0xFFFF;
    TS32(0x64) = d0;
    TS32(0x68) = d1;
}
