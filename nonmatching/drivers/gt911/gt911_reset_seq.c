/*
 * gt911_reset_seq -- stock 0x40049834 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911 power-on reset sequence on PE22/PE24 (selects the I2C address through INT while reset rises).
 */
#include "hw/drivers.h"

#define PE ((pio_port_t *)PIO_PE)

void gt911_reset_seq(void)
{
    gt911_ctrl_pins_out();
    pio_clr_bit(PE, 22);
    pio_clr_bit(PE, 24);
    delay_ms(40);
    pio_set_bit(PE, 22);
    pio_clr_bit(PE, 24);
    delay_ms(40);
    pio_clr_bit(PE, 24);
    delay_ms(40);
    pio_set_bit(PE, 24);
}
