/*
 * sample_divisor_setup -- stock 0x4003A908 (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * From the flags SYS+0x3E / SYS+0x102: mode byte SYS+0x100 (2 when 0x3E is clear, 1 when only 0x3E is set, 0 when both are set), divisor SYS+0x1B0 (2 in the last case, else 1), flag SYS+0x1AC (1 in the last case, else 0), and SYS+0x1B1 = SYS+0x524 / divisor (low byte).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void sample_divisor_setup(void)
{
    u32 mode, div, flag;
    if (SYS8(0x3E) == 0) {
        mode = 2;
        div = 1;
        flag = 0;
    } else if (SYS8(0x102) == 0) {
        mode = 1;
        div = 1;
        flag = 0;
    } else {
        mode = 0;
        div = 2;
        flag = 1;
    }
    SYS8(0x100) = mode;
    SYS8(0x1B0) = div;
    SYS8(0x1AC) = flag;
    SYS8(0x1B1) = SYS32(0x524) / div;
}
