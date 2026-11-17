/*
 * fpga_cmd18_read -- stock 0x40063770 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x18, then SYS+0x279 = bit 0 of one data byte read.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fpga_cmd18_read(void)
{
    fpga_selector_command_write(0x18);
    SYS8(0x279) = fpga_data_byte_read() & 1;
}
