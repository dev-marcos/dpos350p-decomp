/*
 * ms5351_i2c_stop -- stock 0x4005F060 (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M bit-banged I2C STOP.
 */
#include "hw/drivers.h"

#define PE ((pio_port_t *)PIO_PE)

void ms5351_i2c_stop(void)
{
    pio_clr_bit(PE, MS_SCL); delay_us(1);
    pio_clr_bit(PE, MS_SDA); delay_us(1);
    pio_set_bit(PE, MS_SCL); delay_us(1);
    pio_set_bit(PE, MS_SDA); delay_us(1);
}
