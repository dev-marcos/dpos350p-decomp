/*
 * smec_reset_cmd -- stock 0x4004A13C (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes 8 to selector 0x67.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void smec_reset_cmd(void)
{
    fpga_selector_command_write(0x67);
    fpga_data_byte_write(8);
}
