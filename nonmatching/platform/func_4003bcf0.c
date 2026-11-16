/*
 * timebase_index_clamp -- stock 0x4003BCF0 (188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clamps the time-base index SYS+0x588 to [SYS+0x2A8, SYS+0x2A7] (max first, then min). When SYS+0x3C (read first) is set: limits it to the committed index SYS+0x5B0 + 3, then while SYS+0x5B8 / SYS+0x590 / SYS+0x5D4 (64-bit unsigned, after timebase_period_load) >= SYS+0x2AC (16 bit) steps the index up by one and invalidates the windows from id 3 (the stock r1 is the old index).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void ui_window_invalidate_from(u32 id, u32 r1);
void timebase_period_load(void);

void timebase_index_clamp(void)
{
    u32 en = SYS8(0x3C), v, c;
    unsigned long long q;
    v = SYS8(0x2A8);
    if (v < SYS8(0x588))
        v = SYS8(0x588);
    SYS8(0x588) = v;
    v = SYS8(0x2A7);
    if (v > SYS8(0x588))
        v = SYS8(0x588);
    SYS8(0x588) = v;
    if (en == 0)
        return;
    c = SYS8(0x5B0);
    if ((int)(c + 3) < (int)v)
        SYS8(0x588) = c + 3;
    for (;;) {
        u32 lim;
        timebase_period_load();
        q = SYS64(0x5B8) / SYS64(0x590);
        lim = SYS16(0x2AC);
        q /= (u32)SYS32(0x5D4);
        if (q < lim)
            return;
        c = SYS8(0x588);
        SYS8(0x588) = c + 1;
        ui_window_invalidate_from(3, c);
    }
}
