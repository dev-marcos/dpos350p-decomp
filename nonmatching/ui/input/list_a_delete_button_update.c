/*
 * list_a_delete_button_update -- stock 0x4002FB40 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button record +0x7FA of the table 0x42E61FF8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void list_a_delete_button_update(void)
{
    touch_button_update((volatile unsigned char *)0x42E627F2u);
}
