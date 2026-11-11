/*
 * pio_cfg_pin -- stock 0x400631D4 (160 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PIO: set the 4-bit function of a pin (cleared first, then set: 0 clears bit 0 again, 1 sets bit 0, else ORs func) and its 2-bit pull setting.
 */
#include "hw/v3s.h"

void pio_cfg_pin(pio_port_t *port, u32 pin, u32 func, u32 pull)
{
    u32 idx = pin >> 3, sh = (pin << 2) & 0x1C;
    port->cfg[idx] = port->cfg[idx] & ~(0xFu << sh);
    if (func == 0)
        port->cfg[idx] = port->cfg[idx] & ~(1u << sh);
    else if (func == 1)
        port->cfg[idx] = port->cfg[idx] | (1u << sh);
    else
        port->cfg[idx] = port->cfg[idx] | (func << sh);
    volatile u32 *pul = &port->pul[0] + (pin >> 4);
    u32 ps = (pin << 1) & 0x1E;
    *pul = *pul & ~(3u << ps);
    *pul = *pul | (pull << ps);
}
