/*
 * gt911_i2c_start -- stock 0x40049350 (216 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911 bit-banged I2C on PG6 (SDA) / PG7 (SCL): clock pulses, then a START if SDA reads high. Hack FPGA.md lists it as start/bus prepare.
 */
#include "hw/drivers.h"
#include "drivers/gt911.h"
#include "drivers/pio.h"
#include "drivers/timer.h"
#include "shared/drivers.h"

void gt911_i2c_start(void)
{
    pio_set_bit(PG, GT_SCL); delay_us(1);
    pio_clr_bit(PG, GT_SCL); delay_us(1);
    pio_set_bit(PG, GT_SCL); delay_us(1);
    pio_clr_bit(PG, GT_SCL); delay_us(1);
    pio_set_bit(PG, GT_SCL);
    pio_cfg_pin(PG, GT_SDA, 0, 1);
    delay_us(1);
    if (!pio_get_bit(PG, GT_SDA))
        return;
    pio_cfg_pin(PG, GT_SDA, 1, 1);
    pio_clr_bit(PG, GT_SDA); delay_us(1);
    pio_clr_bit(PG, GT_SDA); delay_us(1);
}
