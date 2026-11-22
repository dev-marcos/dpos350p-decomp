/*
 * fpga_bus_init -- stock 0x40049110 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA bus control pins PE0..PE3 as outputs (no pull), PE0 high, PE1 low, then data pins as inputs.
 */
#include "hw/drivers.h"

void fpga_bus_init(void)
{
    pio_port_t *pe = (pio_port_t *)PIO_PE;
    pio_cfg_pin(pe, 1, 1, 0);
    pio_cfg_pin(pe, 2, 1, 0);
    pio_cfg_pin(pe, 3, 1, 0);
    pio_cfg_pin(pe, 0, 1, 0);
    pio_set_bit(pe, 0);
    pio_clr_bit(pe, 1);
    fpga_bus_input();
}
