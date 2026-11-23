/*
 * offset_clamp -- stock 0x4004A7C4 (192 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Keeps the CH1/CH2 positions (SYS_INFO+0x60/+0x120) inside the screen: [m, height - m] with m = SYS_INFO+0x1AE, or [2m, height - 2m] when SYS_INFO+0x2AA is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "frontend.h"
#include "sys_info.h"

void offset_clamp(void)
{
    int m = SI(f_1ae);
    int a = SI(ch1_offset), b = SI(ch2_offset), hgt = SI(plot_height);
    if (SI(zoom_on) == 0) {
        int hi = hgt - m;
        if (hi < a) a = hi;
        else if ((int)SI(f_1ae) >= a) a = SI(f_1ae);
        if (hi < b) b = hi;
        else if (!((int)SI(f_1ae) < b)) b = SI(f_1ae);
    } else {
        int lo = m * 2, hi = hgt - lo;
        if (hi < a) a = hi;
        else if (a < lo) a = lo;
        if (hi < b) b = hi;
        else if (b < lo) b = lo;
    }
    SI(ch1_offset) = a;
    SI(ch2_offset) = b;
}
