/*
 * touch_button_disable -- stock 0x4002F180 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Moves the button off screen: +2, +6 and +4 = 10000 (+8 is left alone).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_button_disable(volatile unsigned char *b)
{
    *(volatile unsigned short *)(b + 2) = 10000;
    *(volatile unsigned short *)(b + 6) = 10000;
    *(volatile unsigned short *)(b + 4) = 10000;
}
