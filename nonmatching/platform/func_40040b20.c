/*
 * second_conversion_wrapper -- stock 0x40040B20 (192 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * As bank_unpack for banks 4..7 (4-way interleave) with an explicit sample count n per lane. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void calibration_gain_baseline_clamp_conversion(u32 b, u32 ch, volatile unsigned short *buf, u32 nb, u32 slot, u32 phase, u32 count);

void second_conversion_wrapper(volatile unsigned short *buf, u32 ch, u32 n)
{
    u32 i;
    for (i = 4; i < 8; i++)
        calibration_gain_baseline_clamp_conversion(SYS8(0x6DC + i), ch, buf, 4, 7 - i, SYS8(0x6D4 + i), n);
}
