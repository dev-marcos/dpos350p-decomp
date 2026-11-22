/*
 * bank_unpack_all -- stock 0x40040BEC (384 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unpacks all 8 banks (bank mode 1 or 2) of the channel into buf, 8-way interleaved: lane i uses the bank number SYS_INFO+0x6DC+i, the phase SYS+0x6D4+i and slot 7 - i; SYS16+0x6F4 samples per lane.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void bank_unpack_all(volatile unsigned short *buf, u32 ch)
{
    u32 i;
    for (i = 0; i < 8; i++)
        calibration_gain_baseline_clamp_conversion(SYS8(0x6DC + i), ch, buf, 8, 7 - i, SYS8(0x6D4 + i), SYS16(0x6F4));
}
