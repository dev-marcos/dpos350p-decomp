/*
 * touch_group_00AC -- stock 0x4002F3C4 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0xAC, +0xB6, +0xC0, +0xCA, +0xD4, +0xDE, +0xE8 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_00AC(void)
{
    touch_button_update((volatile unsigned char *)0x42E620A4u);
    touch_button_update((volatile unsigned char *)0x42E620AEu);
    touch_button_update((volatile unsigned char *)0x42E620B8u);
    touch_button_update((volatile unsigned char *)0x42E620C2u);
    touch_button_update((volatile unsigned char *)0x42E620CCu);
    touch_button_update((volatile unsigned char *)0x42E620D6u);
    touch_button_update((volatile unsigned char *)0x42E620E0u);
}
