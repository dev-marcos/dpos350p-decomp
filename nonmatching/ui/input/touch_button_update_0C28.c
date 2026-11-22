/*
 * touch_button_update_0C28 -- stock 0x40030008 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button record +0xC28 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_button_update_0C28(void)
{
    touch_button_update((volatile unsigned char *)0x42E62C20u);
}
