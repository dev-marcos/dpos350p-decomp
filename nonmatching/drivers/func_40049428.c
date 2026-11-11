/*
 * gt911_i2c_stop -- stock 0x40049428 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911 I2C STOP condition.
 */
#include "hw/drivers.h"

#define PG ((pio_port_t *)PIO_PG)

void gt911_i2c_stop(void)
{
    delay_us(1);
    pio_clr_bit(PG, GT_SCL);
    pio_clr_bit(PG, GT_SDA);
    pio_set_bit(PG, GT_SCL); delay_us(1);
    pio_set_bit(PG, GT_SDA); delay_us(1);
    pio_clr_bit(PG, GT_SCL); delay_us(1);
}
