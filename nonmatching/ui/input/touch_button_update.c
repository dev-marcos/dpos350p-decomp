/*
 * touch_button_update -- stock 0x4002F2BC (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button record b (+0 state, +1 touch byte, +2/+4 x0/y0, +6/+8 x1/y1, all u16): when the current touch point (s16 at 0x42E68BB0+4/+6) lies strictly inside the rectangle, +1 = touch byte +0xD, and +0 = 1 or 2 when the touch state +0xC is 1 (press) or 2 (release).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */
#define TCH16(o) (*(volatile short *)(0x42E68BB0u + (o)))

void touch_button_update(volatile unsigned char *b)
{
    int x = TCH16(4), y;
    if (!((int)*(volatile unsigned short *)(b + 2) < x))
        return;
    if (!(x < (int)*(volatile unsigned short *)(b + 6)))
        return;
    y = TCH16(6);
    if (!((int)*(volatile unsigned short *)(b + 4) < y))
        return;
    if (!(y < (int)*(volatile unsigned short *)(b + 8)))
        return;
    b[1] = TCH8(0xD);
    if (TCH8(0xC) == 1)
        b[0] = 1;
    if (TCH8(0xC) == 2)
        b[0] = 2;
}
