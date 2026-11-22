/*
 * bank_unpack_0_3 -- stock 0x40040D74 (208 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unpacks banks 0..3 (CH1 in bank mode 0) of the channel into buf, 4-way interleaved: lane i uses the bank number SYS_INFO+0x6DC+i, the phase SYS+0x6D4+i and slot 3 - i; SYS16+0x6F4 samples per lane.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void bank_unpack_0_3(volatile unsigned short *buf, u32 ch)
{
    u32 i;
    for (i = 0; i < 4; i++)
        calibration_gain_baseline_clamp_conversion(SYS8(0x6DC + i), ch, buf, 4, 3 - i, SYS8(0x6D4 + i), SYS16(0x6F4));
}
