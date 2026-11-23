/*
 * fpga_cmd61_pulse -- stock 0x400635F4 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x3B = 1 and fpga_cmd61_write; selector 0x17 with data 0; SYS+0x3B = 0 and fpga_cmd61_write.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_cmd61_pulse(void)
{
    SI(fpga_cmd61_value) = 1;
    fpga_cmd61_write();
    fpga_selector_command_write(0x17);
    fpga_data_byte_write(0);
    SI(fpga_cmd61_value) = 0;
    fpga_cmd61_write();
}
