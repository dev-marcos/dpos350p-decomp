/*
 * gt911_wait_ack -- stock 0x40049490 (140 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911 I2C: release SDA, clock high, wait while SDA is high (up to 50000 polls), SDA back to output, SCL low.
 */
#include "hw/drivers.h"
#include "drivers/gt911.h"
#include "drivers/pio.h"
#include "drivers/timer.h"

#define PG ((pio_port_t *)PIO_PG)

void gt911_wait_ack(void)
{
    u32 n = 50000;
    pio_cfg_pin(PG, GT_SDA, 0, 1);
    pio_set_bit(PG, GT_SDA); delay_us(1);
    pio_set_bit(PG, GT_SCL); delay_us(1);
    while (pio_get_bit(PG, GT_SDA) && --n)
        ;
    pio_cfg_pin(PG, GT_SDA, 1, 1);
    pio_clr_bit(PG, GT_SCL);
}
