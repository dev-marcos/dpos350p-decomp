/*
 * touch_pick_b -- stock 0x400692A0 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Like touch_pick_a with touch_hit_pair_b, enable byte 0x42E22D28, codes 0x0E / 0x0F and limit +0x60.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TP      ((volatile short *)0x42E68BB0u)            /* touch block: x0, y0, x1, y1 (s16) */
#define TS32(o) (*(volatile u32 *)(0x42E68998u + (o)))     /* touch-control state */
#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_hit_pair_b(void);

void touch_pick_b(void)
{
    u32 code = 0, best = 0x7FFFFFFF, res = 0;
    if (TP[2] == 0 && TP[3] == 0) {
        TS8 = 0;
        return;
    }
    if (*(volatile unsigned char *)0x42E22D28u != 0) {
        touch_hit_pair_b();
        if (TS32(0x64) < 0x7FFFFFFF) {
            best = TS32(0x64);
            code = 0x0E;
        }
        if (TS32(0x68) < best) {
            code = 0x0F;
            best = TS32(0x68);
        }
    }
    if (best < TS32(0x60))
        res = code;
    TS8 = res;
}
