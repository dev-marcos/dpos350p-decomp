/*
 * touch_group_0002 -- stock 0x4002F324 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x2, +0xC, +0x16, +0x20, +0x2A, +0x34, +0x3E, +0x48, +0x52 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_0002(void)
{
    touch_button_update((volatile unsigned char *)0x42E61FFAu);
    touch_button_update((volatile unsigned char *)0x42E62004u);
    touch_button_update((volatile unsigned char *)0x42E6200Eu);
    touch_button_update((volatile unsigned char *)0x42E62018u);
    touch_button_update((volatile unsigned char *)0x42E62022u);
    touch_button_update((volatile unsigned char *)0x42E6202Cu);
    touch_button_update((volatile unsigned char *)0x42E62036u);
    touch_button_update((volatile unsigned char *)0x42E62040u);
    touch_button_update((volatile unsigned char *)0x42E6204Au);
}
