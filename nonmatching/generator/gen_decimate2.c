/*
 * gen_decimate2 -- stock 0x4004D5CC (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * gen_decimate for the second table (G+0xFB6).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"

void gen_decimate2(int n)
{
    int m;
    unsigned short *s, *d;
    if (n <= 1)
        return;
    m = (int)G16(0x2EF8) / n;
    if (m < 1)
        return;
    s = GEN_BUF2;
    d = GEN_BUF2;
    do {
        *d++ = *s;
        s += n;
    } while (--m);
}
