/*
 * fpga_bus_output -- stock 0x400490F4 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA parallel bus: data pins as outputs (PE_CFG1 = 0x11111111), direction flag = 1.
 */
#include "hw/drivers.h"

void fpga_bus_output(void)
{
    g_fpga_bus_is_output = 1;
    REG32(PIO_PE + 0x04) = 0x11111111u;
}
