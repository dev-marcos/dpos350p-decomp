/*
 * touch_pick_a -- stock 0x40069150 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Control code of a touch: 0 when the second point is (0, 0); otherwise, when 0x42E68668+0x58 is set, the nearer of the two marks of touch_hit_pair_a (0x10 or 0x11) if it is within the limit +0x74, else 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "state/touch_ctl.h"

#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */

void touch_pick_a(void)
{
    u32 code = 0, best = 0x7FFFFFFF, res = 0;
    if (g_touch->down_x == 0 && g_touch->down_y == 0) {
        TS8 = 0;
        return;
    }
    if (*(volatile u32 *)(0x42E68668u + 0x58) != 0) {
        touch_hit_pair_a();
        if (TCS(f_06c) < 0x7FFFFFFF) {
            best = TCS(f_06c);
            code = 0x10;
        }
        if (TCS(f_070) < best) {
            code = 0x11;
            best = TCS(f_070);
        }
    }
    if (best < TCS(f_074))
        res = code;
    TS8 = res;
}
