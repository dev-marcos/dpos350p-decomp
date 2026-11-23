/*
 * measure_select_buttons_update -- stock 0x4002F4F0 (252 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x250, +0x25A, +0x264, +0x26E, +0x278, +0x282, +0x28C, +0x296, +0x2A0, +0x2AA, +0x2B4, +0x2BE, +0x2C8, +0x2D2, +0x2DC, +0x2E6, +0x2F0, +0x2FA, +0x304, +0x30E, +0x318, +0x322, +0x32C, +0x336 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void measure_select_buttons_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E62248u);
    touch_button_update((volatile unsigned char *)0x42E62252u);
    touch_button_update((volatile unsigned char *)0x42E6225Cu);
    touch_button_update((volatile unsigned char *)0x42E62266u);
    touch_button_update((volatile unsigned char *)0x42E62270u);
    touch_button_update((volatile unsigned char *)0x42E6227Au);
    touch_button_update((volatile unsigned char *)0x42E62284u);
    touch_button_update((volatile unsigned char *)0x42E6228Eu);
    touch_button_update((volatile unsigned char *)0x42E62298u);
    touch_button_update((volatile unsigned char *)0x42E622A2u);
    touch_button_update((volatile unsigned char *)0x42E622ACu);
    touch_button_update((volatile unsigned char *)0x42E622B6u);
    touch_button_update((volatile unsigned char *)0x42E622C0u);
    touch_button_update((volatile unsigned char *)0x42E622CAu);
    touch_button_update((volatile unsigned char *)0x42E622D4u);
    touch_button_update((volatile unsigned char *)0x42E622DEu);
    touch_button_update((volatile unsigned char *)0x42E622E8u);
    touch_button_update((volatile unsigned char *)0x42E622F2u);
    touch_button_update((volatile unsigned char *)0x42E622FCu);
    touch_button_update((volatile unsigned char *)0x42E62306u);
    touch_button_update((volatile unsigned char *)0x42E62310u);
    touch_button_update((volatile unsigned char *)0x42E6231Au);
    touch_button_update((volatile unsigned char *)0x42E62324u);
    touch_button_update((volatile unsigned char *)0x42E6232Eu);
}
