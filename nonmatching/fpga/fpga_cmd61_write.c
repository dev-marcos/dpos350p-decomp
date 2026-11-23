/*
 * fpga_cmd61_write -- stock 0x400635D0 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x61, then the data byte SYS+0x3B.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_cmd61_write(void)
{
    fpga_selector_command_write(0x61);
    fpga_data_byte_write(SI(fpga_cmd61_value));
}
