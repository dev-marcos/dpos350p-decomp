/*
 * touch_group_0156 -- stock 0x4002FD80 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x156, +0x160, +0x16A, +0x174 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_0156(void)
{
    touch_button_update((volatile unsigned char *)0x42E6214Eu);
    touch_button_update((volatile unsigned char *)0x42E62158u);
    touch_button_update((volatile unsigned char *)0x42E62162u);
    touch_button_update((volatile unsigned char *)0x42E6216Cu);
}
