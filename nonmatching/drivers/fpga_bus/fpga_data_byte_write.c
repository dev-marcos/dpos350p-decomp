/*
 * fpga_data_byte_write -- stock 0x40049214 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA bus: write one data byte (bus to output if needed; PE2 high, PE3 low; DAT[15:8] = byte; PE0 low then high).
 */
#include "hw/drivers.h"

void fpga_data_byte_write(u32 v)
{
    pio_port_t *pe = (pio_port_t *)PIO_PE;
    if (!g_fpga_bus_is_output)
        fpga_bus_output();
    pio_set_bit(pe, 2);
    pio_clr_bit(pe, 3);
    pe->dat = (pe->dat & ~0xFF00u) | ((v & 0xFF) << 8);
    pio_clr_bit(pe, 0);
    pio_set_bit(pe, 0);
}
