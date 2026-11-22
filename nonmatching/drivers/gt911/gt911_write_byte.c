/*
 * gt911_write_byte -- stock 0x40049554 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911 I2C: shift out one byte MSB first.
 */
#include "hw/drivers.h"
#include "drivers/gt911.h"
#include "drivers/pio.h"
#include "drivers/timer.h"

#define PG ((pio_port_t *)PIO_PG)

void gt911_write_byte(u32 b)
{
    int i = 8;
    pio_clr_bit(PG, GT_SCL);
    do {
        if (b & 0x80) pio_set_bit(PG, GT_SDA);
        else pio_clr_bit(PG, GT_SDA);
        b <<= 1;
        delay_us(1);
        pio_set_bit(PG, GT_SCL); delay_us(1);
        pio_clr_bit(PG, GT_SCL); delay_us(1);
    } while (--i);
}
