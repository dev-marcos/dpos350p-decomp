/*
 * perform_fpga_identification_probe -- stock 0x400492E4 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x01 returns A5 A5 5A 5A on a working FPGA; result in SYS_INFO+0x7FC. (name: Hack FPGA.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void perform_fpga_identification_probe(void)
{
    u32 v;
    fpga_selector_command_write(1);
    v = fpga_data_byte_read();
    v = fpga_data_byte_read() | (v << 8);
    v = fpga_data_byte_read() | (v << 8);
    v = fpga_data_byte_read() | (v << 8);
    SYS8(0x7FC) = v == 0xA5A55A5Au;
}
