/*
 * channel_filters_by_timebase -- stock 0x40041324 (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Frame blend for the enabled channels at the fast time bases: index SYS8+0x588 < 16 uses n = 40 and f = 0.8, 16..20 uses n = 8 and f = 0.7; slower bases are left alone.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

void channel_filters_by_timebase(void)
{
    u32 tb = SYS8(0x588);
    if (tb < 16) {
        if (SYS8(0x3E))
            frame_deviation_blend(0, 0x28, 0.8f);
        if (SYS8(0x102))
            frame_deviation_blend(1, 0x28, 0.8f);
    } else if (tb < 21) {
        if (SYS8(0x3E))
            frame_deviation_blend(0, 8, 0.7f);
        if (SYS8(0x102))
            frame_deviation_blend(1, 8, 0.7f);
    }
}
