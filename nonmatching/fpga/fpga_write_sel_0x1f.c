/*
 * fpga_write_sel_0x1f -- stock 0x4003FA94 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x1F <- v.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fpga_write_sel_0x1f(u32 v)
{
    fpga_selector_command_write(0x1F);
    fpga_data_byte_write(v);
}
