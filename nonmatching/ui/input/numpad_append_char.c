/*
 * numpad_append_char -- stock 0x4005FB94 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Text[+0x17] = c, then the length +0x17 += 1 (no bound check).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

#define PAD(o)  (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define WIN21   ((volatile unsigned short *)(0x42E64520u + 0x21u * 14u))

void numpad_append_char(u32 c)
{
    PAD(3 + PAD(0x17)) = c;
    PAD(0x17) = PAD(0x17) + 1;
}
