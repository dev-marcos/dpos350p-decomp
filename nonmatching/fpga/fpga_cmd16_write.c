/*
 * fpga_cmd16_write -- stock 0x40063460 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x16, then the data byte SYS+0x27B.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_cmd16_write(void)
{
    fpga_selector_command_write(0x16);
    fpga_data_byte_write(SYS8(0x27B));
}
