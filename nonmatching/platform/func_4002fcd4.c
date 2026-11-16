/*
 * touch_group_09F8 -- stock 0x4002FCD4 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x9F8, +0xA02, +0xA0C, +0xA16 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_09F8(void)
{
    touch_button_update((volatile unsigned char *)0x42E629F0u);
    touch_button_update((volatile unsigned char *)0x42E629FAu);
    touch_button_update((volatile unsigned char *)0x42E62A04u);
    touch_button_update((volatile unsigned char *)0x42E62A0Eu);
}
