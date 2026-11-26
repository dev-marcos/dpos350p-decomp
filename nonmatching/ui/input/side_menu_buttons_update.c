/*
 * side_menu_buttons_update -- stock 0x4002F378 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x5C, +0x66, +0x70, +0x7A, +0x84, +0x8E, +0x98, +0xA2 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void side_menu_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E62054u);
    touch_button_update((volatile unsigned char *)0x42E6205Eu);
    touch_button_update((volatile unsigned char *)0x42E62068u);
    touch_button_update((volatile unsigned char *)0x42E62072u);
    touch_button_update((volatile unsigned char *)0x42E6207Cu);
    touch_button_update((volatile unsigned char *)0x42E62086u);
    touch_button_update((volatile unsigned char *)0x42E62090u);
    touch_button_update((volatile unsigned char *)0x42E6209Au);
}
