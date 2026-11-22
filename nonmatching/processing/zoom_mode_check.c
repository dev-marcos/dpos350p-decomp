/*
 * zoom_mode_check -- stock 0x40042350 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears SYS+0x2AA and returns 1 when SYS+0x28 is set with SYS+0x3C clear, when SYS+0x652 is set, or when SYS+0x5B0 equals SYS+0x2A8; otherwise returns 0. Then clamps SYS+0x598 to at most SYS+0x5B0 - 1 (signed compare).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

u32 zoom_mode_check(void)
{
    u32 ret = 0, lim, cur;
    if (SYS8(0x28) && SYS8(0x3C) == 0) {
        SYS8(0x2AA) = 0;
        ret = 1;
    }
    cur = SYS8(0x598);
    if (SYS8(0x652)) {
        SYS8(0x2AA) = 0;
        ret = 1;
    }
    lim = SYS8(0x5B0);
    if (lim == SYS8(0x2A8)) {
        SYS8(0x2AA) = 0;
        ret = 1;
    }
    if ((int)(lim - 1) < (int)cur)
        SYS8(0x598) = lim - 1;
    return ret;
}
