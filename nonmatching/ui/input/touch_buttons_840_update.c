/*
 * touch_buttons_840_update -- stock 0x4002FB98 (152 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the first u16 SYS+0xF9A buttons from 0x42E62838 (10 bytes each, a byte counter) and six more buttons.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

#define TBU(o) touch_button_update((volatile unsigned char *)(0x42E61FF8u + (o)))

void touch_buttons_840_update(void)
{
    unsigned char i;
    for (i = 0; i < SYS16(0xF9A); i++)
        TBU(0x840 + i * 10);
    TBU(0x8D6);
    TBU(0x8E0);
    TBU(0x8EA);
    TBU(0x8F4);
    TBU(0x8FE);
    TBU(0x908);
}
