/*
 * lcd_pins_init -- stock 0x4003E63C (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PD1..PD21 except PD12 to function 2 (LCD RGB / control), no pull.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void lcd_pins_init(void)
{
    u32 pin;
    for (pin = 1; pin < 22; pin++) {
        if (pin == 12)
            continue;
        pio_cfg_pin((void *)0x01C2086Cu, pin, 2, 0);
    }
}
