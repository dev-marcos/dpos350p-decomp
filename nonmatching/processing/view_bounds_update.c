/*
 * view_bounds_update -- stock 0x40042280 (208 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * From the offsets word SYS+0x620 / +0x624 plus u16 SYS+0xC (64-bit sums): the start SYS+0x16 = clamp(a, 0, width SYS+8), the end = b - 1 clamped to 0 .. width - 1 (u16), SYS+0x18 = max(start, end) and SYS+0x1A = SYS+0x18 - start.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void view_bounds_update(void)
{
    long long c = SYS16(0xC), a = (long long)(int)SYS32(0x620) + c, b = (long long)(int)SYS32(0x624) + c;
    u32 lo, hi, m;
    if (a < 0) {
        lo = 0;
    } else {
        u32 w = SYS16(8);
        lo = a < (long long)w ? (u32)a : w;
    }
    SYS16(0x16) = lo;
    if (b < 1) {
        hi = 0;
    } else {
        u32 w = SYS16(8);
        hi = b < (long long)w - 1 ? 0xFFFF + (u32)b : 0xFFFF + w;
    }
    hi = (unsigned short)hi;
    m = SYS16(0x16);
    if (m < hi)
        m = hi;
    SYS16(0x18) = m;
    SYS16(0x1A) = m - SYS16(0x16);
}
