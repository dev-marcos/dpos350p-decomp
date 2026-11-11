/*
 * gt911_read_byte -- stock 0x400495CC (188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911 I2C: shift in one byte MSB first (SDA as input, short busy-wait after each bit), SDA back to output high.
 */
#include "hw/drivers.h"

#define PG ((pio_port_t *)PIO_PG)

u32 gt911_read_byte(void)
{
    u32 v = 0;
    int i = 8;
    pio_clr_bit(PG, GT_SCL);
    pio_cfg_pin(PG, GT_SDA, 0, 1);
    delay_us(1);
    do {
        pio_set_bit(PG, GT_SCL);
        v <<= 1;
        delay_us(1);
        if (pio_get_bit(PG, GT_SDA)) v++;
        pio_clr_bit(PG, GT_SCL);
        delay_us(1);
        for (volatile int k = 0x50; k; k--)     /* stock: 80-iteration NOP loop */
            ;
    } while (--i);
    pio_cfg_pin(PG, GT_SDA, 1, 1);
    pio_set_bit(PG, GT_SDA);
    return v & 0xFF;
}
