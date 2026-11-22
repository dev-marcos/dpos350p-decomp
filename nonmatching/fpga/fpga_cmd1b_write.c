/*
 * fpga_cmd1b_write -- stock 0x400636BC (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x1B, then bits 8..15 and 0..7 of the word SYS+0x288 when SYS+0x275 and +0x274 are set, else two zero bytes.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_cmd1b_write(void)
{
    fpga_selector_command_write(0x1B);
    if (SYS8(0x275) && SYS8(0x274)) {
        fpga_data_byte_write((SYS32(0x288) >> 8) & 0xFF);
        fpga_data_byte_write(SYS32(0x288) & 0xFF);
    } else {
        fpga_data_byte_write(0);
        fpga_data_byte_write(0);
    }
}
