/*
 * touch_group_03C2 -- stock 0x4002F684 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x3C2, +0x3CC, +0x3D6, +0x3E0, +0x3EA, +0x3F4, +0x3FE, +0x408, +0x412, +0x41C of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_group_03C2(void)
{
    touch_button_update((volatile unsigned char *)0x42E623BAu);
    touch_button_update((volatile unsigned char *)0x42E623C4u);
    touch_button_update((volatile unsigned char *)0x42E623CEu);
    touch_button_update((volatile unsigned char *)0x42E623D8u);
    touch_button_update((volatile unsigned char *)0x42E623E2u);
    touch_button_update((volatile unsigned char *)0x42E623ECu);
    touch_button_update((volatile unsigned char *)0x42E623F6u);
    touch_button_update((volatile unsigned char *)0x42E62400u);
    touch_button_update((volatile unsigned char *)0x42E6240Au);
    touch_button_update((volatile unsigned char *)0x42E62414u);
}
