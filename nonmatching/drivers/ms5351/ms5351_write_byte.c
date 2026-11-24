/*
 * ms5351_write_byte -- stock 0x4005F0BC (168 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M: shift out one byte MSB first, then clock the ACK slot without reading it.
 */
#include "hw/drivers.h"
#include "drivers/ms5351.h"
#include "drivers/pio.h"
#include "drivers/timer.h"
#include "shared/drivers.h"

void ms5351_write_byte(u32 b)
{
    int i = 0;
    do {
        pio_clr_bit(PE, MS_SCL);
        if (b & 0x80) pio_set_bit(PE, MS_SDA);
        else pio_clr_bit(PE, MS_SDA);
        i++;
        delay_us(1);
        pio_set_bit(PE, MS_SCL);
        b <<= 1;
        delay_us(1);
    } while (i < 8);
    pio_clr_bit(PE, MS_SCL);
    pio_set_bit(PE, MS_SDA);
    delay_us(1);
    pio_set_bit(PE, MS_SCL);
    delay_us(1);
    pio_clr_bit(PE, MS_SCL);
}
