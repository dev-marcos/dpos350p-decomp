/*
 * fpga_bus_input -- stock 0x400490DC (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA parallel bus (PE8..PE15): data pins as inputs (PE_CFG1 = 0), direction flag = 0. Hack: FPGA_PROTOCOL.md.
 */
#include "hw/drivers.h"

void fpga_bus_input(void)
{
    g_fpga_bus_is_output = 0;
    REG32(PIO_PE + 0x04) = 0;
}
