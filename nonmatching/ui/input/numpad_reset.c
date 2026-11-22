/*
 * numpad_reset -- stock 0x4005FB4C (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Entry pad state 0x42E68A88: the 20-byte text +3 cleared, +0 (16 bit) = 0, length +0x17 = 0, the '.'/'-' flags +0x18 (16 bit) = 0, value +0x20 (double) = 0.0, and the pointers +0x28 -> +0x38, +0x2C -> +0x40, +0x30 -> +0x48.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define PAD(o)  (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define WIN21   ((volatile unsigned short *)(0x42E64520u + 0x21u * 14u))

void mem_clear(void *d, unsigned int n);

void numpad_reset(void)
{
    mem_clear((void *)0x42E68A8Bu, 0x14);
    *(volatile unsigned short *)0x42E68A88u = 0;
    PAD(0x17) = 0;
    *(volatile unsigned short *)0x42E68AA0u = 0;
    *(volatile double *)0x42E68AA8u = 0.0;
    *(volatile u32 *)0x42E68AB4u = 0x42E68AC8u;
    *(volatile u32 *)0x42E68AB8u = 0x42E68AD0u;
    *(volatile u32 *)0x42E68AB0u = 0x42E68AC0u;
}
