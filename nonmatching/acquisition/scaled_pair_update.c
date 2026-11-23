/*
 * scaled_pair_update -- stock 0x4003C0AC (172 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * With n0 = SYS+0x5E8 + 1, n1 = SYS+0x5EC - 1 (signed) and k = SYS+0x5D4: when mode_not_0_or_1(mode) is 0, SYS+0x608 = n0 x k / SYS+0x5F8 and SYS+0x60C = n1 x k / SYS+0x5F8; otherwise the same with the divisor SYS+0x610 into SYS+0x620 / SYS+0x624 (64-bit signed products and divisions).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void scaled_pair_update(u32 mode)
{
    long long p = (long long)(int)(SI(f_5e8) + 1) * (long long)(u32)SI(f_5d4);
    long long k;
    if (mode_not_0_or_1(mode) == 0) {
        long long d = (long long)(u32)SI(f_5f8);
        k = (long long)(u32)SI(f_5d4);
        SI(f_608) = (u32)(p / d);
        p = (long long)(int)(SI(f_5ec) - 1) * k;
        SI(f_60c) = (u32)(p / (long long)(u32)SI(f_5f8));
    } else {
        long long d = (long long)(u32)SI(f_610);
        k = (long long)(u32)SI(f_5d4);
        SI(f_620) = (u32)(p / d);
        p = (long long)(int)(SI(f_5ec) - 1) * k;
        SI(f_624) = (u32)(p / (long long)(u32)SI(f_610));
    }
}
