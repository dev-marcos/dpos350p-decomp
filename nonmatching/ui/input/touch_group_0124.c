/*
 * touch_group_0124 -- stock 0x4002FD3C (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x124, +0x12E, +0x138, +0x142, +0x14C of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_0124(void)
{
    touch_button_update((volatile unsigned char *)0x42E6211Cu);
    touch_button_update((volatile unsigned char *)0x42E62126u);
    touch_button_update((volatile unsigned char *)0x42E62130u);
    touch_button_update((volatile unsigned char *)0x42E6213Au);
    touch_button_update((volatile unsigned char *)0x42E62144u);
}
