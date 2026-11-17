/*
 * cursor_808_clamp -- stock 0x4003407C (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Words SYS+0x808 and +0x80C are kept in 12..(u16 SYS+8) - 12 (the first is always stored, the second only when changed).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void cursor_808_clamp(void)
{
    int v = (int)SYS32(0x808), m;
    if (v < 12) {
        v = 12;
    } else {
        m = SYS16(8) - 12;
        if (m < v)
            v = m;
    }
    SYS32(0x808) = v;
    v = (int)SYS32(0x80C);
    if (v < 12) {
        SYS32(0x80C) = 12;
    } else {
        m = SYS16(8) - 12;
        if (m < v)
            SYS32(0x80C) = m;
    }
}
