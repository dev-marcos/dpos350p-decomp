/*
 * touch_group_0BE2 -- stock 0x4002FFAC (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0xBE2, +0xBEC, +0xBF6, +0xC00, +0xC0A, +0xC14, +0xC1E of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_0BE2(void)
{
    touch_button_update((volatile unsigned char *)0x42E62BDAu);
    touch_button_update((volatile unsigned char *)0x42E62BE4u);
    touch_button_update((volatile unsigned char *)0x42E62BEEu);
    touch_button_update((volatile unsigned char *)0x42E62BF8u);
    touch_button_update((volatile unsigned char *)0x42E62C02u);
    touch_button_update((volatile unsigned char *)0x42E62C0Cu);
    touch_button_update((volatile unsigned char *)0x42E62C16u);
}
