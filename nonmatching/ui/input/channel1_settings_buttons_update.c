/*
 * channel1_settings_buttons_update -- stock 0x4002F614 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x35E, +0x368, +0x372, +0x37C, +0x386, +0x390, +0x39A, +0x3A4, +0x3AE, +0x3B8 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void channel1_settings_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E62356u);
    touch_button_update((volatile unsigned char *)0x42E62360u);
    touch_button_update((volatile unsigned char *)0x42E6236Au);
    touch_button_update((volatile unsigned char *)0x42E62374u);
    touch_button_update((volatile unsigned char *)0x42E6237Eu);
    touch_button_update((volatile unsigned char *)0x42E62388u);
    touch_button_update((volatile unsigned char *)0x42E62392u);
    touch_button_update((volatile unsigned char *)0x42E6239Cu);
    touch_button_update((volatile unsigned char *)0x42E623A6u);
    touch_button_update((volatile unsigned char *)0x42E623B0u);
}
