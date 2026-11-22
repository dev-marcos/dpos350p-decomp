/*
 * touch_buttons_728_update -- stock 0x4002FAAC (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the first u16 SYS+0xF9A buttons from 0x42E62720 (10 bytes each, a byte counter) and six more buttons.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

#define TBU(o) touch_button_update((volatile unsigned char *)(0x42E61FF8u + (o)))

void touch_buttons_728_update(void)
{
    unsigned char i;
    for (i = 0; i < SYS16(0xF9A); i++)
        TBU(0x728 + i * 10);
    TBU(0x7BE);
    TBU(0x7C8);
    TBU(0x7D2);
    TBU(0x7DC);
    TBU(0x7E6);
    TBU(0x7F0);
}
