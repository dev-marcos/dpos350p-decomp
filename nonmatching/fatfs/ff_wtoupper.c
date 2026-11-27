/*
 * ff_wtoupper -- stock 0x40069E38 (272 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unicode upper-case conversion (FatFs R0.13b ff_wtoupper, tables at 0x400877AC and 0x4008C6E0). Values of 0x10000 and above pass unchanged. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

u32 ff_wtoupper(u32 uni)
{
    const volatile unsigned short *p;
    u32 uc, bc, nc, cmd;
    if (uni >= 0x10000)
        return uni;
    uc = uni & 0xFFFF;
    p = uc < 0x1000 ? (const volatile unsigned short *)IMG(400877ac) : (const volatile unsigned short *)IMG(4008c6e0);
    for (;;) {
        bc = *p++;
        if (bc == 0 || uc < bc)
            break;
        nc = *p++;
        cmd = nc >> 8;
        nc &= 0xFF;
        if (uc < bc + nc) {
            switch (cmd) {
            case 0: uc = p[uc - bc]; break;
            case 1: uc -= (uc - bc) & 1; break;
            case 2: uc -= 16; break;
            case 3: uc -= 32; break;
            case 4: uc -= 48; break;
            case 5: uc -= 26; break;
            case 6: uc += 8; break;
            case 7: uc -= 80; break;
            case 8: uc -= 0x1C60; break;
            }
            break;
        }
        if (cmd == 0)
            p += nc;
    }
    return uc & 0xFFFF;
}
