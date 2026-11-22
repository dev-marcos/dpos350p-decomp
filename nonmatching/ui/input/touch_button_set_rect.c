/*
 * touch_button_set_rect -- stock 0x4002F160 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button rectangle from x, y, w, h: +2 = x, +6 = x + w, +4 = y, +8 = y + h (16 bit); the record is the fifth (stack) argument.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b)
{
    *(volatile unsigned short *)(b + 2) = x;
    *(volatile unsigned short *)(b + 6) = x + wd;
    *(volatile unsigned short *)(b + 4) = y;
    *(volatile unsigned short *)(b + 8) = y + ht;
}
