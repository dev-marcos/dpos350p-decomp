/*
 * touch_pick_b -- stock 0x400692A0 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Like touch_pick_a with touch_hit_pair_b, enable byte 0x42E22D28, codes 0x0E / 0x0F and limit +0x60.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "state/touch_ctl.h"

#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_pick_b(void)
{
    u32 code = 0, best = 0x7FFFFFFF, res = 0;
    if (g_touch->down_x == 0 && g_touch->down_y == 0) {
        TS8 = 0;
        return;
    }
    if (*(volatile unsigned char *)0x42E22D28u != 0) {
        touch_hit_pair_b();
        if (TCS(f_064) < 0x7FFFFFFF) {
            best = TCS(f_064);
            code = 0x0E;
        }
        if (TCS(f_068) < best) {
            code = 0x0F;
            best = TCS(f_068);
        }
    }
    if (best < TCS(f_060))
        res = code;
    TS8 = res;
}
