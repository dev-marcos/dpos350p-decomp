/*
 * table_scatter_42cf64b4 -- stock 0x4003CA38 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears 0x3AAC bytes at 0x42CF64B4, then for the 0x3AAC (value, index) halfword pairs at 0x40090444 stores value at 0x42CF64B4 + 2 x index.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "util.h"

void table_scatter_42cf64b4(void)
{
    volatile unsigned short *d = (volatile unsigned short *)0x42CF64B4u;
    const volatile unsigned short *s = (const volatile unsigned short *)0x40090444u;
    u32 n;
    memset_w((void *)d, 0, 0x3AAC);
    for (n = 0x3AAC; n; n--) {
        d[s[1]] = s[0];
        s += 2;
    }
}
