/*
 * fpga_write_sel_0x2c -- stock 0x4004ABA8 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA selector 0x2C <- SYS_INFO+0x10B.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fpga_write_sel_0x2c(void)
{
    fpga_selector_command_write(0x2C);
    fpga_data_byte_write(SYS8(0x10B));
}
