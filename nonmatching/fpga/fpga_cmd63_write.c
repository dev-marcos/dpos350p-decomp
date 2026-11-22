/*
 * fpga_cmd63_write -- stock 0x4004EEEC (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x63, then the data byte byte 0x42E428E4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_cmd63_write(void)
{
    fpga_selector_command_write(0x63);
    fpga_data_byte_write(*(volatile unsigned char *)0x42E428E4u);
}
