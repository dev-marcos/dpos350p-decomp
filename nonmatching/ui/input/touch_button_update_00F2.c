/*
 * touch_button_update_00F2 -- stock 0x4002F408 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button record +0xF2 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_button_update_00F2(void)
{
    touch_button_update((volatile unsigned char *)0x42E620EAu);
}
