/*
 * write_sys_info_0x6f0_to_fpga -- stock 0x4003F910 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x09 <- SYS_INFO+0x6F0 (16 bit, high byte first). (name: Hack ACQUISITION.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void write_sys_info_0x6f0_to_fpga(void)
{
    fpga_selector_command_write(9);
    fpga_data_byte_write(SYS16(0x6F0) >> 8);
    fpga_data_byte_write(SYS16(0x6F0) & 0xFF);
}
