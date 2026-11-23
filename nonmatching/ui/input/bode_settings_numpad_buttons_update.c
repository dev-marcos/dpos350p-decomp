/*
 * bode_settings_numpad_buttons_update -- stock 0x4002FE98 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0xB06, +0xB10, +0xB1A, +0xB24, +0xB2E, +0xB38 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void bode_settings_numpad_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E62AFEu);
    touch_button_update((volatile unsigned char *)0x42E62B08u);
    touch_button_update((volatile unsigned char *)0x42E62B12u);
    touch_button_update((volatile unsigned char *)0x42E62B1Cu);
    touch_button_update((volatile unsigned char *)0x42E62B26u);
    touch_button_update((volatile unsigned char *)0x42E62B30u);
}
