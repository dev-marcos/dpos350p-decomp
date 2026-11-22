/*
 * fpga_cmd15_write -- stock 0x40063430 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x15, then 1 when SYS+0x27C is set, else 2.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fpga_cmd15_write(void)
{
    fpga_selector_command_write(0x15);
    fpga_data_byte_write(SYS8(0x27C) ? 1 : 2);
}
