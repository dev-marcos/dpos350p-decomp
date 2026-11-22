/*
 * cursor_ac64_clamp -- stock 0x40037A60 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Block 0x42E180C8: words +0xAC64 and +0xAC68 are kept in 2..(u16 +4) - 2.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

void cursor_ac64_clamp(void)
{
    volatile unsigned char *b = (volatile unsigned char *)0x42E180C8u;
    volatile int *a = (volatile int *)(b + 0xAC64);
    int lim = *(volatile unsigned short *)(b + 4) - 2, x, y;
    if (a[0] < 2)
        a[0] = 2;
    y = a[1];
    x = a[0];
    if (y < 2)
        y = 2;
    if (lim <= x)
        x = lim;
    a[0] = x;
    a[1] = lim > y ? y : lim;
}
