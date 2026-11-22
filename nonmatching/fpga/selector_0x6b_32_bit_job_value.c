/*
 * selector_0x6b_32_bit_job_value -- stock 0x40049F7C (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Job value, MSB first (selector 0x6B). (name: Hack SMEC.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void selector_0x6b_32_bit_job_value(u32 v)
{
    fpga_selector_command_write(0x6B);
    fpga_data_byte_write(v >> 24);
    fpga_data_byte_write((v << 8) >> 24);
    fpga_data_byte_write((v << 16) >> 24);
    fpga_data_byte_write(v & 0xFF);
}
