/*
 * touch_group_0AC0 -- stock 0x4002FE3C (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0xAC0, +0xACA, +0xAD4, +0xADE, +0xAE8, +0xAF2, +0xAFC of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_group_0AC0(void)
{
    touch_button_update((volatile unsigned char *)0x42E62AB8u);
    touch_button_update((volatile unsigned char *)0x42E62AC2u);
    touch_button_update((volatile unsigned char *)0x42E62ACCu);
    touch_button_update((volatile unsigned char *)0x42E62AD6u);
    touch_button_update((volatile unsigned char *)0x42E62AE0u);
    touch_button_update((volatile unsigned char *)0x42E62AEAu);
    touch_button_update((volatile unsigned char *)0x42E62AF4u);
}
