/*
 * touch_group_0106 -- stock 0x4002FD10 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x106, +0x110, +0x11A of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_0106(void)
{
    touch_button_update((volatile unsigned char *)0x42E620FEu);
    touch_button_update((volatile unsigned char *)0x42E62108u);
    touch_button_update((volatile unsigned char *)0x42E62112u);
}
