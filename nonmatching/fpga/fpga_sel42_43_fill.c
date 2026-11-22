/*
 * fpga_sel42_43_fill -- stock 0x4004EC98 (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA: selector 0x42 = 1, then 130 pairs (1, 0) to selector 0x43; selector 0x42 = 0, then n pairs (1, 0) to selector 0x43.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_sel42_43_fill(u32 n)
{
    int i;
    fpga_selector_command_write(0x42);
    fpga_data_byte_write(1);
    fpga_selector_command_write(0x43);
    for (i = 0x82; i; i--) {
        fpga_data_byte_write(1);
        fpga_data_byte_write(0);
    }
    fpga_selector_command_write(0x42);
    fpga_data_byte_write(0);
    fpga_selector_command_write(0x43);
    for (; n; n--) {
        fpga_data_byte_write(1);
        fpga_data_byte_write(0);
    }
}
