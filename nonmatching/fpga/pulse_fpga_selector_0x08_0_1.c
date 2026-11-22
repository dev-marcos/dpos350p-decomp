/*
 * pulse_fpga_selector_0x08_0_1 -- stock 0x4003F8E0 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x08: 0 then 1 (runtime re-arm). (name: Hack ACQUISITION.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void pulse_fpga_selector_0x08_0_1(void)
{
    fpga_selector_command_write(8);
    fpga_data_byte_write(0);
    fpga_selector_command_write(8);
    fpga_data_byte_write(1);
}
