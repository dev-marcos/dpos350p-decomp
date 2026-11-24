/*
 * fpga_write_sel_0x2a -- stock 0x4004AA94 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA selector 0x2A <- SYS_INFO+0x44 (16 bit, high byte first: CH1 sweep/DAC value).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_write_sel_0x2a(void)
{
    fpga_selector_command_write(0x2A);
    fpga_data_byte_write(SI(ch1_fpga_2a) >> 8);
    fpga_data_byte_write(SI(ch1_fpga_2a) & 0xFF);
}
