/*
 * runtime_counter_progress_advance -- stock 0x4003BF98 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sample counter step: when SYS+0x6FC >= SYS+0x708, adds SYS+0x6F8 (16 bit) to SYS+0x700 and SYS+0x56C; then SYS+0x6FC += SYS+0x6F6 (16 bit), q = SYS+0x6FC / SYS+0x1B1 (0 when the divisor is 0, as the udiv instruction gives), SYS+0x63C = q, SYS+0x704 = remainder, SYS+0x6F0 (16 bit) = SYS+0x6F4 + q.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void runtime_counter_progress_advance(void)
{
    u32 c = SYS32(0x6FC), d, q;
    if (c >= SYS32(0x708)) {
        u32 step = SYS16(0x6F8);
        SYS32(0x700) = SYS32(0x700) + step;
        SYS32(0x56C) = SYS32(0x56C) + step;
    }
    c += SYS16(0x6F6);
    d = SYS8(0x1B1);
    SYS32(0x6FC) = c;
    q = d ? c / d : 0;
    SYS32(0x63C) = q;
    SYS32(0x704) = c - d * q;
    SYS16(0x6F0) = SYS16(0x6F4) + q;
}
