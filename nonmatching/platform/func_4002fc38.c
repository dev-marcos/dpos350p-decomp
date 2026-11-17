/*
 * touch_buttons_91c_update -- stock 0x4002FC38 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the first u16 SYS+0xF9A buttons from 0x42E62914 (10 bytes each, a byte counter) and six more buttons.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);
#define TBU(o) touch_button_update((volatile unsigned char *)(0x42E61FF8u + (o)))

void touch_buttons_91c_update(void)
{
    unsigned char i;
    for (i = 0; i < SYS16(0xF9A); i++)
        TBU(0x91C + i * 10);
    TBU(0x9B2);
    TBU(0x9BC);
    TBU(0x9C6);
    TBU(0x9D0);
    TBU(0x9DA);
    TBU(0x9E4);
}
