/*
 * touch_group_0426 -- stock 0x4002F6FC (368 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button records +0x426, +0x430, +0x43A, +0x444, +0x44E, +0x458, +0x462, +0x46C, +0x476, +0x480, +0x48A, +0x494, +0x49E, +0x4A8, +0x4B2, +0x4BC, +0x4C6, +0x4D0, +0x4DA, +0x4E4, +0x4EE, +0x4F8, +0x502, +0x50C, +0x516, +0x520, +0x52A, +0x534, +0x53E, +0x548, +0x552 of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);

void touch_group_0426(void)
{
    touch_button_update((volatile unsigned char *)0x42E6241Eu);
    touch_button_update((volatile unsigned char *)0x42E62428u);
    touch_button_update((volatile unsigned char *)0x42E62432u);
    touch_button_update((volatile unsigned char *)0x42E6243Cu);
    touch_button_update((volatile unsigned char *)0x42E62446u);
    touch_button_update((volatile unsigned char *)0x42E62450u);
    touch_button_update((volatile unsigned char *)0x42E6245Au);
    touch_button_update((volatile unsigned char *)0x42E62464u);
    touch_button_update((volatile unsigned char *)0x42E6246Eu);
    touch_button_update((volatile unsigned char *)0x42E62478u);
    touch_button_update((volatile unsigned char *)0x42E62482u);
    touch_button_update((volatile unsigned char *)0x42E6248Cu);
    touch_button_update((volatile unsigned char *)0x42E62496u);
    touch_button_update((volatile unsigned char *)0x42E624A0u);
    touch_button_update((volatile unsigned char *)0x42E624AAu);
    touch_button_update((volatile unsigned char *)0x42E624B4u);
    touch_button_update((volatile unsigned char *)0x42E624BEu);
    touch_button_update((volatile unsigned char *)0x42E624C8u);
    touch_button_update((volatile unsigned char *)0x42E624D2u);
    touch_button_update((volatile unsigned char *)0x42E624DCu);
    touch_button_update((volatile unsigned char *)0x42E624E6u);
    touch_button_update((volatile unsigned char *)0x42E624F0u);
    touch_button_update((volatile unsigned char *)0x42E624FAu);
    touch_button_update((volatile unsigned char *)0x42E62504u);
    touch_button_update((volatile unsigned char *)0x42E6250Eu);
    touch_button_update((volatile unsigned char *)0x42E62518u);
    touch_button_update((volatile unsigned char *)0x42E62522u);
    touch_button_update((volatile unsigned char *)0x42E6252Cu);
    touch_button_update((volatile unsigned char *)0x42E62536u);
    touch_button_update((volatile unsigned char *)0x42E62540u);
    touch_button_update((volatile unsigned char *)0x42E6254Au);
}
