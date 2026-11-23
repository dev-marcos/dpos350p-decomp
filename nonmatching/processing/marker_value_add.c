/*
 * marker_value_add -- stock 0x40030060 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Adds v (negated when mode is 1) to the 24 s16 values at SYS+0x880 unless it is already there: increments the counter SYS+0x800 (mode 0) or SYS+0x801 (other modes), then stores v in the first zero slot (if any).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void marker_value_add(u32 mode, int v)
{
    volatile short *t = (volatile short *)(0x40544E68u + 0x880u);
    int i;
    if (mode == 1)
        v = -v;
    for (i = 0; i < 24; i++)
        if (t[i] == v)
            return;
    if (mode == 0)
        SI(f_800)++;
    else
        SI(f_801)++;
    for (i = 0; i < 24; i++) {
        if (t[i] == 0) {
            t[i] = v;
            return;
        }
    }
}
