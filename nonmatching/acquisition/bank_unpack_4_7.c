/*
 * bank_unpack_4_7 -- stock 0x40040E44 (208 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unpacks banks 4..7 (CH2 in bank mode 0) of the channel into buf, 4-way interleaved: lane i uses the bank number SYS_INFO+0x6DC+i, the phase SYS+0x6D4+i and slot 7 - i; SYS16+0x6F4 samples per lane.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void bank_unpack_4_7(volatile unsigned short *buf, u32 ch)
{
    u32 i;
    for (i = 4; i < 8; i++)
        calibration_gain_baseline_clamp_conversion(SYS8(0x6DC + i), ch, buf, 4, 7 - i, SYS8(0x6D4 + i), SYS16(0x6F4));
}
