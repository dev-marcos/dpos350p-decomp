/*
 * fpga_write_sel_0x2d -- stock 0x4004ABCC (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA selector 0x2D <- SYS_INFO+0x108 (16 bit, high byte first: CH2 counterpart of 0x2A).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_write_sel_0x2d(void)
{
    fpga_selector_command_write(0x2D);
    fpga_data_byte_write(SI(ch2_fpga_2d) >> 8);
    fpga_data_byte_write(SI(ch2_fpga_2d) & 0xFF);
}
