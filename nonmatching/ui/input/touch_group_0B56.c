/*
 * touch_group_0B56 -- stock 0x4002FEF8 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0xB56, +0xB60, +0xB6A, +0xB74, +0xB7E, +0xB88 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_group_0B56(void)
{
    touch_button_update((volatile unsigned char *)0x42E62B4Eu);
    touch_button_update((volatile unsigned char *)0x42E62B58u);
    touch_button_update((volatile unsigned char *)0x42E62B62u);
    touch_button_update((volatile unsigned char *)0x42E62B6Cu);
    touch_button_update((volatile unsigned char *)0x42E62B76u);
    touch_button_update((volatile unsigned char *)0x42E62B80u);
}
