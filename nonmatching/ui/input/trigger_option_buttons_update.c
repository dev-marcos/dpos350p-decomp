/*
 * trigger_option_buttons_update -- stock 0x4002F8E0 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x5B6, +0x5C0, +0x5CA, +0x5D4, +0x5DE, +0x5E8, +0x5F2, +0x5FC, +0x606, +0x610, +0x61A of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void trigger_option_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E625AEu);
    touch_button_update((volatile unsigned char *)0x42E625B8u);
    touch_button_update((volatile unsigned char *)0x42E625C2u);
    touch_button_update((volatile unsigned char *)0x42E625CCu);
    touch_button_update((volatile unsigned char *)0x42E625D6u);
    touch_button_update((volatile unsigned char *)0x42E625E0u);
    touch_button_update((volatile unsigned char *)0x42E625EAu);
    touch_button_update((volatile unsigned char *)0x42E625F4u);
    touch_button_update((volatile unsigned char *)0x42E625FEu);
    touch_button_update((volatile unsigned char *)0x42E62608u);
    touch_button_update((volatile unsigned char *)0x42E62612u);
}
