/*
 * numpad_key_buttons_update -- stock 0x4002FDCC (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for 10 buttons from 0x42E62A18 and six more.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

#define TBU(o) touch_button_update((volatile unsigned char *)(0x42E61FF8u + (o)))

void numpad_key_buttons_update(void)
{
    int i;
    for (i = 0; i < 10; i++)
        TBU(0xA20 + i * 10);
    TBU(0xA84);
    TBU(0xA8E);
    TBU(0xA98);
    TBU(0xAA2);
    TBU(0xAAC);
    TBU(0xAB6);
}
