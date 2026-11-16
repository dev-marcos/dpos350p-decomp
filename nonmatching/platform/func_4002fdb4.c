/*
 * touch_button_update_01EC -- stock 0x4002FDB4 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button record +0x1EC of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_button_update_01EC(void)
{
    touch_button_update((volatile unsigned char *)0x42E621E4u);
}
