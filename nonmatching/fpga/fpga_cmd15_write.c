/*
 * fpga_cmd15_write -- stock 0x40063430 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x15, then 1 when SYS+0x27C is set, else 2.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_cmd15_write(void)
{
    fpga_selector_command_write(0x15);
    fpga_data_byte_write(SI(trigger_channel) ? 1 : 2);
}
