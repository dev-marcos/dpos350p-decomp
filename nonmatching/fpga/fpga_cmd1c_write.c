/*
 * fpga_cmd1c_write -- stock 0x40063714 (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x1C, then bits 8..15 and 0..7 of the word SYS+0x28C (read before the selector) when SYS+0x275 and +0x274 are set, else two zero bytes.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_cmd1c_write(void)
{
    u32 v = SYS32(0x28C);
    fpga_selector_command_write(0x1C);
    if (SYS8(0x275) && SYS8(0x274)) {
        fpga_data_byte_write((v >> 8) & 0xFF);
        fpga_data_byte_write(v & 0xFF);
    } else {
        fpga_data_byte_write(0);
        fpga_data_byte_write(0);
    }
}
