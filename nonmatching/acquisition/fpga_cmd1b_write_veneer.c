/*
 * fpga_cmd1b_write_veneer -- stock 0x4003FAB0 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Branch to 0x400636BC.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"

void fpga_cmd1b_write_veneer(void)
{
    fpga_cmd1b_write();
}
