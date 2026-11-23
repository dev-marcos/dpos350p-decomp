/*
 * system_settings_buttons_update -- stock 0x4002F410 (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x17E, +0xFC, +0x192, +0x1A6, +0x1B0, +0x1BA, +0x1C4, +0x1CE, +0x1D8, +0x1E2, +0x1F6 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void system_settings_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E62176u);
    touch_button_update((volatile unsigned char *)0x42E620F4u);
    touch_button_update((volatile unsigned char *)0x42E6218Au);
    touch_button_update((volatile unsigned char *)0x42E6219Eu);
    touch_button_update((volatile unsigned char *)0x42E621A8u);
    touch_button_update((volatile unsigned char *)0x42E621B2u);
    touch_button_update((volatile unsigned char *)0x42E621BCu);
    touch_button_update((volatile unsigned char *)0x42E621C6u);
    touch_button_update((volatile unsigned char *)0x42E621D0u);
    touch_button_update((volatile unsigned char *)0x42E621DAu);
    touch_button_update((volatile unsigned char *)0x42E621EEu);
}
