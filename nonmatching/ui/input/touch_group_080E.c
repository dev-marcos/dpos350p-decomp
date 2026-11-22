/*
 * touch_group_080E -- stock 0x4002FB50 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x80E, +0x818, +0x822, +0x82C, +0x836 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_080E(void)
{
    touch_button_update((volatile unsigned char *)0x42E62806u);
    touch_button_update((volatile unsigned char *)0x42E62810u);
    touch_button_update((volatile unsigned char *)0x42E6281Au);
    touch_button_update((volatile unsigned char *)0x42E62824u);
    touch_button_update((volatile unsigned char *)0x42E6282Eu);
}
