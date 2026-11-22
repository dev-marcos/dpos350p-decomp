/*
 * array_3da548_decimate -- stock 0x400374EC (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For n > 1: keeps every n-th u16 of the array SYS+0x3DA548, packing (word SYS+0x3DA53C) / n of them at its start.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

void array_3da548_decimate(int n)
{
    volatile unsigned short *d, *s;
    int c;
    if (n <= 1)
        return;
    d = s = (volatile unsigned short *)(0x40544E68u + 0x3DA548u);
    c = (int)SYS32(0x3DA53C) / n;
    for (; c >= 1; c--) {
        *d++ = *s;
        s += n;
    }
}
