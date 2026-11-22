/*
 * fpga_write_sel_0x30 -- stock 0x4004ACA4 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA selector 0x30 <- SYS_INFO+0x103.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fpga_write_sel_0x30(void)
{
    fpga_selector_command_write(0x30);
    fpga_data_byte_write(SYS8(0x103));
}
