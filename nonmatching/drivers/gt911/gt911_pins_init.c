/*
 * gt911_pins_init -- stock 0x40049A5C (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911 I2C and control pins.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/gt911.h"

void gt911_pins_init(void)
{
    gt911_i2c_pins_init();
    gt911_ctrl_pins_out();
}
