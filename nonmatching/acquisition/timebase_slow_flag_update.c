/*
 * timebase_slow_flag_update -- stock 0x4003BE78 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x28 = 1 when the time-base index SYS+0x588 >= SYS+0x2A9, else 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void timebase_slow_flag_update(void)
{
    SYS8(0x28) = SYS8(0x588) >= SYS8(0x2A9) ? 1 : 0;
}
