/*
 * bank_unpack_0_3_n -- stock 0x40040A5C (192 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * As bank_unpack for banks 0..3 (4-way interleave) with an explicit sample count n per lane.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void calibration_gain_baseline_clamp_conversion(u32 b, u32 ch, volatile unsigned short *buf, u32 nb, u32 slot, u32 phase, u32 count);

void bank_unpack_0_3_n(volatile unsigned short *buf, u32 ch, u32 n)
{
    u32 i;
    for (i = 0; i < 4; i++)
        calibration_gain_baseline_clamp_conversion(SYS8(0x6DC + i), ch, buf, 4, 3 - i, SYS8(0x6D4 + i), n);
}
