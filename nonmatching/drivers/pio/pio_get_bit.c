/*
 * pio_get_bit -- stock 0x400632A0 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PIO: (DAT >> pin) & 1.
 */
#include "hw/v3s.h"

int pio_get_bit(pio_port_t *port, u32 pin)
{
    return (lsr32(port->dat, pin) & 1) ? 1 : 0;
}
