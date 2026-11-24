/*
 * gt911_send_ack -- stock 0x4004951C (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911 I2C: drive an ACK bit (SDA low, one SCL pulse).
 */
#include "hw/drivers.h"
#include "drivers/gt911.h"
#include "drivers/pio.h"
#include "drivers/timer.h"
#include "shared/drivers.h"

void gt911_send_ack(void)
{
    pio_clr_bit(PG, GT_SDA);
    pio_set_bit(PG, GT_SCL);
    delay_us(1);
    pio_clr_bit(PG, GT_SCL);
}
