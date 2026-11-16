/*
 * zoom_index_sub -- stock 0x40042470 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS+0x614 < SYS16+0x2AC x SYS+0x554 and d < SYS+0x598: SYS+0x598 -= d. Then 0x400423B8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_400423b8(void);

void zoom_index_sub(u32 d)
{
    u32 b;
    if (SYS32(0x614) < SYS16(0x2AC) * SYS32(0x554)) {
        b = SYS8(0x598);
        if (d < b)
            SYS8(0x598) = b - d;
    }
    func_400423b8();
}
