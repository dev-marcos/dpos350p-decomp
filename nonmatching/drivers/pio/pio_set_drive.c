/*
 * pio_set_drive -- stock 0x400632B8 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PIO: set the 2-bit drive strength of a pin (values above 3 clamp to 3).
 */
#include "hw/v3s.h"
#include "drivers/pio.h"

void pio_set_drive(pio_port_t *port, u32 pin, u32 drv)
{
    volatile u32 *r = &port->drv[0] + (pin >> 4);
    u32 sh = (pin << 1) & 0x1E;
    if (drv > 3) drv = 3;
    *r = ~(3u << sh) & *r;
    *r = *r | (drv << sh);
}
