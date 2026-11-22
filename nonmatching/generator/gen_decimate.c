/*
 * gen_decimate -- stock 0x4004D350 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Keeps every n-th sample of the table G+0xE (count G+0x2EF8 / n), n > 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"

void gen_decimate(int n)
{
    int m;
    unsigned short *s, *d;
    if (n <= 1)
        return;
    m = (int)G16(0x2EF8) / n;
    if (m < 1)
        return;
    s = GEN_BUF;
    d = GEN_BUF;
    do {
        *d++ = *s;
        s += n;
    } while (--m);
}
