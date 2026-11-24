/*
 * zoom_reset -- stock 0x400424D4 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * timebase_index_set(SYS+0x5B0), zoom_index_set(SYS+0x5B0 - 1), sys_word_pair_570_set(SYS64+0x5E0), copies SYS+0x570/574 to +0x578/57C, then sys_word_pair_578_set(0, 0) (so +0x578/57C end as 0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"
#include "sys_info.h"

void zoom_reset(void)
{
    u32 lo, hi;
    timebase_index_set(SI(committed_timebase_index));
    zoom_index_set((SI(committed_timebase_index) - 1) & 0xFF);
    sys_word_pair_570_set(SYS32(0x5E0), SYS32(0x5E4));
    lo = SYS32(0x570);
    hi = SYS32(0x574);
    SYS32(0x578) = lo;
    SYS32(0x57C) = hi;
    sys_word_pair_578_set(0, 0);
}
