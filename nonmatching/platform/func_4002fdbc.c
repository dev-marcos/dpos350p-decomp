/*
 * touch_button_update_0200 -- stock 0x4002FDBC (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button record +0x200 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_button_update_0200(void)
{
    touch_button_update((volatile unsigned char *)0x42E621F8u);
}
