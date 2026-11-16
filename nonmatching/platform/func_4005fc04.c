/*
 * numpad_sign_apply -- stock 0x4005FC04 (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Makes the text agree with the '-' flag +0x19: flag set and no leading '-': shift text +3..+0x15 right by one, put '-' at +3, length + 1; flag clear and a leading '-': shift +4..+0x16 left by one, length - 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define PAD(o)  (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define WIN21   ((volatile unsigned short *)(0x42E64520u + 0x21u * 14u))

void numpad_sign_apply(void)
{
    int i;
    int c = (signed char)PAD(3);
    if (PAD(0x19)) {
        if (c == '-')
            return;
        for (i = 0x15; i >= 3; i--)
            PAD(i + 1) = PAD(i);
        PAD(3) = '-';
        PAD(0x17) = PAD(0x17) + 1;
    } else {
        if (c != '-')
            return;
        for (i = 3; i < 0x16; i++)
            PAD(i) = PAD(i + 1);
        PAD(0x17) = PAD(0x17) - 1;
    }
}
