/*
 * zoom_mode_check -- stock 0x40042350 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears SYS+0x2AA and returns 1 when SYS+0x28 is set with SYS+0x3C clear, when SYS+0x652 is set, or when SYS+0x5B0 equals SYS+0x2A8; otherwise returns 0. Then clamps SYS+0x598 to at most SYS+0x5B0 - 1 (signed compare).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

u32 zoom_mode_check(void)
{
    u32 ret = 0, lim, cur;
    if (SI(acq_path_mode) && SI(acq_transition_flag) == 0) {
        SI(zoom_on) = 0;
        ret = 1;
    }
    cur = SI(zoom_index);
    if (SI(f_652)) {
        SI(zoom_on) = 0;
        ret = 1;
    }
    lim = SI(committed_timebase_index);
    if (lim == SI(timebase_index_min)) {
        SI(zoom_on) = 0;
        ret = 1;
    }
    if ((int)(lim - 1) < (int)cur)
        SI(zoom_index) = lim - 1;
    return ret;
}
