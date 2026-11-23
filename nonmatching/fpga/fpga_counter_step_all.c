/*
 * fpga_counter_step_all -- stock 0x40063CB0 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40063C34(i) for i = 0..7.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"

void fpga_counter_step_all(void)
{
    u32 i;
    for (i = 0; i < 8; i++)
        fpga_counter_step(i & 0xFF);
}
