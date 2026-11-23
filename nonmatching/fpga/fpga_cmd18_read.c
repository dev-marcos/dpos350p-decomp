/*
 * fpga_cmd18_read -- stock 0x40063770 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x18, then SYS+0x279 = bit 0 of one data byte read.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_cmd18_read(void)
{
    fpga_selector_command_write(0x18);
    SI(f_279) = fpga_data_byte_read() & 1;
}
