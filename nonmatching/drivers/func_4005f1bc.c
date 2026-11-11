/*
 * ms5351_pins_init -- stock 0x4005F1BC (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M: PE20/PE21 as outputs (no pull). Hack: MS5351M path.
 */
#include "hw/drivers.h"

#define PE ((pio_port_t *)PIO_PE)

void ms5351_pins_init(void)
{
    pio_cfg_pin(PE, MS_SCL, 1, 0);
    pio_cfg_pin(PE, MS_SDA, 1, 0);
}
