/*
 * fpga_trigger_pos_write -- stock 0x4003F988 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x20 <- position (16 bit, high byte first), then strobe 0x21: 0, 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_trigger_pos_write(u32 v)
{
    fpga_selector_command_write(0x20);
    fpga_data_byte_write((v << 16) >> 24);
    fpga_data_byte_write(v & 0xFF);
    fpga_selector_command_write(0x21);
    fpga_data_byte_write(0);
    fpga_selector_command_write(0x21);
    fpga_data_byte_write(1);
}
