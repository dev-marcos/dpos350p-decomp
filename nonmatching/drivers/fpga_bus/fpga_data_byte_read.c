/*
 * fpga_data_byte_read -- stock 0x400491B8 (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA bus: read one data byte (bus to input if needed; PE2, PE3, PE0 low; PE0 high; byte = DAT[15:8]). Hack: FPGA.md.
 */
#include "hw/drivers.h"
#include "drivers/fpga_bus.h"
#include "drivers/pio.h"

u32 fpga_data_byte_read(void)
{
    pio_port_t *pe = (pio_port_t *)PIO_PE;
    if (g_fpga_bus_is_output)
        fpga_bus_input();
    pio_clr_bit(pe, 2);
    pio_clr_bit(pe, 3);
    pio_clr_bit(pe, 0);
    pio_set_bit(pe, 0);
    return (pe->dat << 16) >> 24;
}
