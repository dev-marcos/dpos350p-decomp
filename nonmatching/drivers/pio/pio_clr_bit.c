/*
 * pio_clr_bit -- stock 0x40063288 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PIO: DAT &= ~(1 << pin).
 */
#include "hw/v3s.h"

void pio_clr_bit(pio_port_t *port, u32 pin)
{
    port->dat = ~(1u << pin) & port->dat;
}
