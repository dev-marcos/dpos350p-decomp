/*
 * timebase_index_clamp -- stock 0x4003BCF0 (188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clamps the time-base index SYS+0x588 to [SYS+0x2A8, SYS+0x2A7] (max first, then min). When SYS+0x3C (read first) is set: limits it to the committed index SYS+0x5B0 + 3, then while SYS+0x5B8 / SYS+0x590 / SYS+0x5D4 (64-bit unsigned, after timebase_period_load) >= SYS+0x2AC (16 bit) steps the index up by one and invalidates the windows from id 3 (the stock r1 is the old index).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "ui/windows.h"
#include "sys_info.h"

void timebase_index_clamp(void)
{
    u32 en = SI(acq_transition_flag), v, c;
    unsigned long long q;
    v = SI(timebase_index_min);
    if (v < SI(timebase_index))
        v = SI(timebase_index);
    SI(timebase_index) = v;
    v = SI(timebase_index_max);
    if (v > SI(timebase_index))
        v = SI(timebase_index);
    SI(timebase_index) = v;
    if (en == 0)
        return;
    c = SI(f_5b0);
    if ((int)(c + 3) < (int)v)
        SI(timebase_index) = c + 3;
    for (;;) {
        u32 lim;
        timebase_period_load();
        q = SI(f_5b8) / SI(f_590);
        lim = SI(f_2ac);
        q /= (u32)SI(f_5d4);
        if (q < lim)
            return;
        c = SI(timebase_index);
        SI(timebase_index) = c + 1;
        ui_window_invalidate_from(3, c);
    }
}
