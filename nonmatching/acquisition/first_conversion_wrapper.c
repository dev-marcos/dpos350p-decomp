/*
 * first_conversion_wrapper -- stock 0x400408FC (352 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * As bank_unpack for banks 0..7 (8-way interleave) with an explicit sample count n per lane. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void first_conversion_wrapper(volatile unsigned short *buf, u32 ch, u32 n)
{
    u32 i;
    for (i = 0; i < 8; i++)
        calibration_gain_baseline_clamp_conversion(SYS8(0x6DC + i), ch, buf, 8, 7 - i, SYS8(0x6D4 + i), n);
}
