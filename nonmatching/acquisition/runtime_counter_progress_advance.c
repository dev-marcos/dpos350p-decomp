/*
 * runtime_counter_progress_advance -- stock 0x4003BF98 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sample counter step: when SYS+0x6FC >= SYS+0x708, adds SYS+0x6F8 (16 bit) to SYS+0x700 and SYS+0x56C; then SYS+0x6FC += SYS+0x6F6 (16 bit), q = SYS+0x6FC / SYS+0x1B1 (0 when the divisor is 0, as the udiv instruction gives), SYS+0x63C = q, SYS+0x704 = remainder, SYS+0x6F0 (16 bit) = SYS+0x6F4 + q.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void runtime_counter_progress_advance(void)
{
    u32 c = SI(runtime_fine_progress), d, q;
    if (c >= SI(runtime_progress_threshold)) {
        u32 step = SI(runtime_coarse_step);
        SI(runtime_coarse_progress) = SI(runtime_coarse_progress) + step;
        SI(f_56c) = SI(f_56c) + step;
    }
    c += SI(runtime_step);
    d = SI(repeat_factor);
    SI(runtime_fine_progress) = c;
    q = d ? c / d : 0;
    SI(runtime_quotient) = q;
    SI(runtime_field_704) = c - d * q;
    SI(runtime_fpga_position) = SI(runtime_field_6f4) + q;
}
