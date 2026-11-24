/*
 * sample_divisor_setup -- stock 0x4003A908 (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * From the flags SYS+0x3E / SYS+0x102: mode byte SYS+0x100 (2 when 0x3E is clear, 1 when only 0x3E is set, 0 when both are set), divisor SYS+0x1B0 (2 in the last case, else 1), flag SYS+0x1AC (1 in the last case, else 0), and SYS+0x1B1 = SYS+0x524 / divisor (low byte).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void sample_divisor_setup(void)
{
    u32 mode, div, flag;
    if (SI(ch1_enabled) == 0) {
        mode = 2;
        div = 1;
        flag = 0;
    } else if (SI(ch2_enabled) == 0) {
        mode = 1;
        div = 1;
        flag = 0;
    } else {
        mode = 0;
        div = 2;
        flag = 1;
    }
    SI(bank_mode) = mode;
    SI(sample_divisor) = div;
    SI(f_1ac) = flag;
    SI(repeat_factor) = SI(sample_rate) / div;
}
