/*
 * vscale_update -- stock 0x4004A608 (444 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Vertical scale per channel: table[V/div] x probe factor (x1, x10, x100 for probe 0/1/2) into SYS_INFO+0x50 (CH1) / +0x110 (CH2); a second value from table[SYS_INFO+0x1B9 - 1] into +0xD8 / +0x198. Other probe values leave them unchanged.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void vscale_update(void)
{
    u32 p1 = SYS8(0x40), p2 = SYS8(0x104);
    unsigned long long k, v198;
    if (p1 <= 2) {
        k = p1 == 0 ? 1 : p1 == 1 ? 10 : 100;
        SYS64(0x50) = SYS64(0xEF0 + 8 * SYS8(0x41)) * k;
        SYS64(0xD8) = SYS64(0xEE8 + 8 * SYS8(0x1B9)) * k;
    }
    v198 = SYS64(0x198);
    if (p2 > 2) {
        SYS32(0x198) = (u32)v198;
        SYS32(0x19C) = (u32)(v198 >> 32);
        return;
    }
    k = p2 == 0 ? 1 : p2 == 1 ? 10 : 100;
    SYS64(0x110) = SYS64(0xEF0 + 8 * SYS8(0x105)) * k;
    v198 = SYS64(0xEE8 + 8 * SYS8(0x1B9)) * k;
    SYS32(0x198) = (u32)v198;
    SYS32(0x19C) = (u32)(v198 >> 32);
}
