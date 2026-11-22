/*
 * gt911_send_ack -- stock 0x4004951C (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911 I2C: drive an ACK bit (SDA low, one SCL pulse).
 */
#include "hw/drivers.h"

#define PG ((pio_port_t *)PIO_PG)

void gt911_send_ack(void)
{
    pio_clr_bit(PG, GT_SDA);
    pio_set_bit(PG, GT_SCL);
    delay_us(1);
    pio_clr_bit(PG, GT_SCL);
}
