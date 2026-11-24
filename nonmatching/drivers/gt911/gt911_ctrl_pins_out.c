/*
 * gt911_ctrl_pins_out -- stock 0x400497D4 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911: PE22 and PE24 (reset / INT) as outputs, both high.
 */
#include "hw/drivers.h"
#include "drivers/gt911.h"
#include "drivers/pio.h"
#include "shared/drivers.h"

void gt911_ctrl_pins_out(void)
{
    pio_cfg_pin(PE, 22, 1, 0);
    pio_cfg_pin(PE, 24, 1, 0);
    pio_set_bit(PE, 22);
    pio_set_bit(PE, 24);
}
