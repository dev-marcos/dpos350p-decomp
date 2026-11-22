/*
 * offset_clamp -- stock 0x4004A7C4 (192 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Keeps the CH1/CH2 positions (SYS_INFO+0x60/+0x120) inside the screen: [m, height - m] with m = SYS_INFO+0x1AE, or [2m, height - 2m] when SYS_INFO+0x2AA is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "frontend.h"

void offset_clamp(void)
{
    int m = SYS16(0x1AE);
    int a = SYS32(0x60), b = SYS32(0x120), hgt = SYS16(0x0A);
    if (SYS8(0x2AA) == 0) {
        int hi = hgt - m;
        if (hi < a) a = hi;
        else if ((int)SYS16(0x1AE) >= a) a = SYS16(0x1AE);
        if (hi < b) b = hi;
        else if (!((int)SYS16(0x1AE) < b)) b = SYS16(0x1AE);
    } else {
        int lo = m * 2, hi = hgt - lo;
        if (hi < a) a = hi;
        else if (a < lo) a = lo;
        if (hi < b) b = hi;
        else if (b < lo) b = lo;
    }
    SYS32(0x60) = a;
    SYS32(0x120) = b;
}
