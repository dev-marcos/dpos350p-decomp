/*
 * spi0_pins_init -- stock 0x4004341C (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When mode is 0: PC0..PC3 to function 3 (SPI0), drive level 3.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void spi0_pins_init(u32 mode)
{
    u32 pin;
    if (mode != 0)
        return;
    for (pin = 0; pin < 4; pin++) {
        pio_cfg_pin((void *)0x01C20848u, pin, 3, 0);
        pio_set_drive((void *)0x01C20848u, pin, 3);
    }
}
