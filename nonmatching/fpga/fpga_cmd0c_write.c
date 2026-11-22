/*
 * fpga_cmd0c_write -- stock 0x40063354 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x0C, then the u16 SYS+0x276, high byte first.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_cmd0c_write(void)
{
    fpga_selector_command_write(0x0C);
    fpga_data_byte_write(SYS16(0x276) >> 8);
    fpga_data_byte_write(SYS16(0x276) & 0xFF);
}
