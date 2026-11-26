/*
 * colour_column_buttons_update -- stock 0x4002FCD4 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x9F8, +0xA02, +0xA0C, +0xA16 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void colour_column_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E629F0u);
    touch_button_update((volatile unsigned char *)0x42E629FAu);
    touch_button_update((volatile unsigned char *)0x42E62A04u);
    touch_button_update((volatile unsigned char *)0x42E62A0Eu);
}
