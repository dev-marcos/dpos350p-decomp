/*
 * numpad_sign_apply -- stock 0x4005FC04 (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Makes the text agree with the '-' flag +0x19: flag set and no leading '-': shift text +3..+0x15 right by one, put '-' at +3, length + 1; flag clear and a leading '-': shift +4..+0x16 left by one, length - 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "state/entry_pad.h"

#define PAD(o)  (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define WIN21   ((volatile unsigned short *)(0x42E64520u + 0x21u * 14u))

void numpad_sign_apply(void)
{
    int i;
    int c = (signed char)NUMPAD(f_003);
    if (NUMPAD(has_minus)) {
        if (c == '-')
            return;
        for (i = 0x15; i >= 3; i--)
            PAD(i + 1) = PAD(i);
        NUMPAD(f_003) = '-';
        NUMPAD(len) = NUMPAD(len) + 1;
    } else {
        if (c != '-')
            return;
        for (i = 3; i < 0x16; i++)
            PAD(i) = PAD(i + 1);
        NUMPAD(len) = NUMPAD(len) - 1;
    }
}
