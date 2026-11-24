/*
 * gt911_i2c_pins_init -- stock 0x40049788 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911: PG6/PG7 as outputs with pull-up, both high (bus idle).
 */
#include "hw/drivers.h"
#include "drivers/gt911.h"
#include "drivers/pio.h"
#include "shared/drivers.h"

void gt911_i2c_pins_init(void)
{
    pio_cfg_pin(PG, GT_SDA, 1, 1);
    pio_cfg_pin(PG, GT_SCL, 1, 1);
    pio_set_bit(PG, GT_SDA);
    pio_set_bit(PG, GT_SCL);
}
