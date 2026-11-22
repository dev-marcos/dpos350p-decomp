/*
 * timebase_period_load -- stock 0x4003A8BC (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x590 (64 bit) = the table entry SYS+0xBD0 + 8 x SYS+0x588 (64 bit).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void timebase_period_load(void)
{
    SYS64(0x590) = SYS64(0xBD0 + 8u * SYS8(0x588));
}
