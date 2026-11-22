/*
 * touch_group_0340 -- stock 0x4002F5EC (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x340, +0x34A, +0x354 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_0340(void)
{
    touch_button_update((volatile unsigned char *)0x42E62338u);
    touch_button_update((volatile unsigned char *)0x42E62342u);
    touch_button_update((volatile unsigned char *)0x42E6234Cu);
}
