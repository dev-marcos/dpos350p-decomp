/*
 * gt911_int_input -- stock 0x40049820 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911: PE24 (INT) back to input.
 */
#include "hw/drivers.h"

void gt911_int_input(void)
{
    pio_cfg_pin((pio_port_t *)PIO_PE, 24, 0, 0);
}
