/*
 * touch_button_update_019C -- stock 0x4002FDC4 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button record +0x19C of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_button_update_019C(void)
{
    touch_button_update((volatile unsigned char *)0x42E62194u);
}
