/*
 * touch_buttons_6xx_update -- stock 0x4002F968 (316 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * touch_button_update for the button at 0x42E62644, then for 9 buttons (0x42E6261C..) when byte 0x42E3F990 is 0, otherwise for 15 others (0x42E62680..).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_button_update(volatile unsigned char *b);
#define TBU(o) touch_button_update((volatile unsigned char *)(0x42E61FF8u + (o)))

void touch_buttons_6xx_update(void)
{
    TBU(0x64C);
    if (*(volatile unsigned char *)(0x42E3F984u + 0xC) == 0) {
        TBU(0x624);
        TBU(0x62E);
        TBU(0x638);
        TBU(0x642);
        TBU(0x656);
        TBU(0x660);
        TBU(0x66A);
        TBU(0x674);
        TBU(0x67E);
    } else {
        TBU(0x688);
        TBU(0x692);
        TBU(0x69C);
        TBU(0x6A6);
        TBU(0x6B0);
        TBU(0x6BA);
        TBU(0x6C4);
        TBU(0x6CE);
        TBU(0x6D8);
        TBU(0x6E2);
        TBU(0x6EC);
        TBU(0x6F6);
        TBU(0x700);
        TBU(0x70A);
        TBU(0x714);
    }
}
