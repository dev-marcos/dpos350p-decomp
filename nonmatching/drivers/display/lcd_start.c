/*
 * lcd_start -- stock 0x4003E688 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TCON and DE clocks off, LCD pins, then display_init(1022, 600, 0x41FB8FE8).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/display.h"

void lcd_start(void)
{
    tcon_clock_off();
    de_clock_off();
    lcd_pins_init();
    display_init(0x3FE, 0x258, 0x41FB8FE8u);
}
