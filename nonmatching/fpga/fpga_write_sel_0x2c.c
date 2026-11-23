/*
 * fpga_write_sel_0x2c -- stock 0x4004ABA8 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA selector 0x2C <- SYS_INFO+0x10B.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_write_sel_0x2c(void)
{
    fpga_selector_command_write(0x2C);
    fpga_data_byte_write(SI(ch2_bandwidth_filter));
}
