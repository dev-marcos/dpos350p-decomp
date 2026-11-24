/*
 * touch_hit_pair_a -- stock 0x400690E4 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Horizontal distance from the touch x to two marks (0x42E68668: +0x5C/+0x60 plus offset +0xB0, as u16); a distance above the limit (touch state +0x74) becomes 0xFFFF. Stored at +0x6C/+0x70.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "state/touch_ctl.h"

#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_hit_pair_a(void)
{
    volatile u32 *b = (volatile u32 *)0x42E68668u;
    int x = g_touch->down_x;
    int a = (unsigned short)(b[0x5C / 4] + b[0xB0 / 4]);
    int c = (unsigned short)(b[0x60 / 4] + b[0xB0 / 4]);
    u32 d0 = x >= a ? (u32)(x - a) : (u32)(a - x);
    u32 d1 = x >= c ? (u32)(x - c) : (u32)(c - x);
    if (TCS(f_074) < d0) d0 = 0xFFFF;
    if (TCS(f_074) < d1) d1 = 0xFFFF;
    TCS(f_06c) = d0;
    TCS(f_070) = d1;
}
