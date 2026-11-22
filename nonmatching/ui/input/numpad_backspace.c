/*
 * numpad_backspace -- stock 0x4005FBB4 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Length -1 when not 0; the character now at the end clears the flag +0x19 when it is '-' or +0x18 when it is '.', and is replaced by NUL.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

#define PAD(o)  (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define WIN21   ((volatile unsigned short *)(0x42E64520u + 0x21u * 14u))

void numpad_backspace(void)
{
    int c;
    if (PAD(0x17))
        PAD(0x17) = PAD(0x17) - 1;
    c = (signed char)PAD(3 + PAD(0x17));
    if (c == '-')
        PAD(0x19) = 0;
    else if (c == '.')
        PAD(0x18) = 0;
    PAD(3 + PAD(0x17)) = 0;
}
