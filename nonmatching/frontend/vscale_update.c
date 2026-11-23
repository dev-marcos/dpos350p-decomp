/*
 * vscale_update -- stock 0x4004A608 (444 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Vertical scale per channel: table[V/div] x probe factor (x1, x10, x100 for probe 0/1/2) into SYS_INFO+0x50 (CH1) / +0x110 (CH2); a second value from table[SYS_INFO+0x1B9 - 1] into +0xD8 / +0x198. Other probe values leave them unchanged.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "frontend.h"
#include "sys_info.h"

void vscale_update(void)
{
    u32 p1 = SI(ch1_probe), p2 = SI(ch2_probe);
    unsigned long long k, v198;
    if (p1 <= 2) {
        k = p1 == 0 ? 1 : p1 == 1 ? 10 : 100;
        SI(ch1_scale) = SYS64(0xEF0 + 8 * SI(ch1_vdiv_index)) * k;
        SI(f_0d8) = SYS64(0xEE8 + 8 * SI(f_1b9)) * k;
    }
    v198 = SYS64(0x198);
    if (p2 > 2) {
        SI(f_198) = (u32)v198;
        SI(f_19c) = (u32)(v198 >> 32);
        return;
    }
    k = p2 == 0 ? 1 : p2 == 1 ? 10 : 100;
    SI(ch2_scale) = SYS64(0xEF0 + 8 * SI(ch2_vdiv_index)) * k;
    v198 = SYS64(0xEE8 + 8 * SI(f_1b9)) * k;
    SI(f_198) = (u32)v198;
    SI(f_19c) = (u32)(v198 >> 32);
}
