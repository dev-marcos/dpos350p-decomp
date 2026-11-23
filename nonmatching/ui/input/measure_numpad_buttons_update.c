/*
 * measure_numpad_buttons_update -- stock 0x4002FF48 (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0xB92, +0xB9C, +0xBA6, +0xBB0, +0xBBA, +0xBC4, +0xBCE of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void measure_numpad_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E62B8Au);
    touch_button_update((volatile unsigned char *)0x42E62B94u);
    touch_button_update((volatile unsigned char *)0x42E62B9Eu);
    touch_button_update((volatile unsigned char *)0x42E62BA8u);
    touch_button_update((volatile unsigned char *)0x42E62BB2u);
    touch_button_update((volatile unsigned char *)0x42E62BBCu);
    touch_button_update((volatile unsigned char *)0x42E62BC6u);
}
