/*
 * display_menu_buttons_update -- stock 0x4002F490 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x20A, +0x214, +0x21E, +0x228, +0x17E, +0x232, +0x23C, +0x246 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void display_menu_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E62202u);
    touch_button_update((volatile unsigned char *)0x42E6220Cu);
    touch_button_update((volatile unsigned char *)0x42E62216u);
    touch_button_update((volatile unsigned char *)0x42E62220u);
    touch_button_update((volatile unsigned char *)0x42E62176u);
    touch_button_update((volatile unsigned char *)0x42E6222Au);
    touch_button_update((volatile unsigned char *)0x42E62234u);
    touch_button_update((volatile unsigned char *)0x42E6223Eu);
}
