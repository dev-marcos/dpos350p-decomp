/*
 * ms5351_i2c_start -- stock 0x4005F014 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M bit-banged I2C START.
 */
#include "hw/drivers.h"
#include "drivers/ms5351.h"
#include "drivers/pio.h"
#include "drivers/timer.h"
#include "shared/drivers.h"

void ms5351_i2c_start(void)
{
    pio_set_bit(PE, MS_SDA);
    pio_set_bit(PE, MS_SCL);
    pio_clr_bit(PE, MS_SDA);
    delay_us(1);
    pio_clr_bit(PE, MS_SCL);
    delay_us(1);
}
