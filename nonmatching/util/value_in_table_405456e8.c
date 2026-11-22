/*
 * value_in_table_405456e8 -- stock 0x400301A8 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 1 when v (negated when mode is 1) equals one of the 24 s16 at 0x405456E8, else 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "util.h"

u32 value_in_table_405456e8(u32 mode, int v)
{
    volatile short *t = (volatile short *)0x405456E8u;
    int n;
    if (mode == 1)
        v = -v;
    for (n = 24; n; n--)
        if (*t++ == v)
            return 1;
    return 0;
}
