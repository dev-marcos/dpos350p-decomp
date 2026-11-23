/*
 * output_select_buttons_update -- stock 0x4002F86C (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x55C, +0x566, +0x570, +0x57A, +0x584, +0x58E, +0x598, +0x5A2, +0x5AC of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void output_select_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E62554u);
    touch_button_update((volatile unsigned char *)0x42E6255Eu);
    touch_button_update((volatile unsigned char *)0x42E62568u);
    touch_button_update((volatile unsigned char *)0x42E62572u);
    touch_button_update((volatile unsigned char *)0x42E6257Cu);
    touch_button_update((volatile unsigned char *)0x42E62586u);
    touch_button_update((volatile unsigned char *)0x42E62590u);
    touch_button_update((volatile unsigned char *)0x42E6259Au);
    touch_button_update((volatile unsigned char *)0x42E625A4u);
}
