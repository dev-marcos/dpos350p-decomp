/*
 * fpga_selector_command_write -- stock 0x4004927C (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA bus: write a command/selector byte (as data write but with PE2 and PE3 high).
 */
#include "hw/drivers.h"

void fpga_selector_command_write(u32 v)
{
    pio_port_t *pe = (pio_port_t *)PIO_PE;
    if (!g_fpga_bus_is_output)
        fpga_bus_output();
    pio_set_bit(pe, 2);
    pio_set_bit(pe, 3);
    pe->dat = (pe->dat & ~0xFF00u) | ((v & 0xFF) << 8);
    pio_clr_bit(pe, 0);
    pio_set_bit(pe, 0);
}
