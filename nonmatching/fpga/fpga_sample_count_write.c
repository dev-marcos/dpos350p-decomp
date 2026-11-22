/*
 * fpga_sample_count_write -- stock 0x4003F948 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x0E <- n - 2 (32 bit, MSB first).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_sample_count_write(u32 n)
{
    u32 v = n - 2;
    fpga_selector_command_write(0x0E);
    fpga_data_byte_write((u32)((int)v >> 24) & 0xFF);
    fpga_data_byte_write((v << 8) >> 24);
    fpga_data_byte_write((v << 16) >> 24);
    fpga_data_byte_write(v & 0xFF);
}
