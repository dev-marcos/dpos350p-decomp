/*
 * retained_inside_current_sc1_3_t6 -- stock 0x4004C544 (368 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Calibration coefficient selector: aux values for the CH1/CH2 V/div (object+0..+0xC) and one matrix row (min(SYS+0x588, SYS+0x6ED) - SYS+0x6EC) into the active coefficients A/B (object+0x10..+0x4C): columns 16..31 when SYS_INFO+0x100 is 0, else 0..15. (name: Hack STOCK_LIKE_MINIMAL_RUNTIME.md working label)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "sys_info.h"

void retained_inside_current_sc1_3_t6(void)
{
    volatile u32 *o = CAL_OBJ;
    u32 i1 = SI(ch1_vdiv_index), i2, a, b, c, row, base, j;
    volatile u32 *p;
    o[0] = o[0x50 / 4 + i1];
    o[2] = o[0xF0 / 4 + i1];
    i2 = SI(ch2_vdiv_index);
    o[1] = o[0xA0 / 4 + i2];
    o[3] = o[0x140 / 4 + i2];
    a = SI(timebase_index);
    b = SI(f_6ed);
    c = SI(f_6ec);
    row = (b >= a) ? a - c : b - c;
    p = (volatile u32 *)((u32)CAL_OBJ + row * 4);
    base = SI(f_100) == 0 ? 16 : 0;
    for (j = 0; j < 16; j++)
        o[4 + j] = p[(0x190 + (base + j) * 0xC8) / 4];
}
