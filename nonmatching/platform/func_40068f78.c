/*
 * strcmp -- stock 0x40068F78 (160 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Byte-wise string compare (the stock code compares a word at a time when it can); returns the difference of the first unequal unsigned bytes.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int strcmp(const char *a, const char *b)
{
    const volatile unsigned char *p = (const volatile unsigned char *)a, *q = (const volatile unsigned char *)b;
    u32 c, d;
    do {
        c = *p++;
        d = *q++;
    } while (c >= 1 && c == d);
    return (int)(c - d);
}
