/*
 * channel_minmax_stats -- stock 0x40032A58 (228 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For channel ch (0 needs SYS+0x3E, 1 needs SYS+0x102): over the samples (u16 array at word SYS+0x7B4 + 4ch) from h = (SYS+0x2B4 - SYS+0x2B0) / 2 up to SYS+0x2B4 - h, the maximum and minimum, their mean and the range, stored in SYS+0xB60/B64/B6C/B68 (ch 0) or SYS+0xB88/B8C/B94/B90.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void channel_minmax_stats(u32 ch)
{
    volatile unsigned short *p = (volatile unsigned short *)SYS32(0x7B4 + 4 * ch);
    u32 omax, omin, omid, orng;
    int h, end, i, mx = 0, mn = 0xFFFFFF;
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
        omax = 0xB60; omin = 0xB64; omid = 0xB6C; orng = 0xB68;
    } else {
        if (ch == 1 && SYS8(0x102) == 0)
            return;
        omax = 0xB88; omin = 0xB8C; omid = 0xB94; orng = 0xB90;
    }
    h = (u32)(SYS32(0x2B4) - SYS32(0x2B0)) >> 1;
    end = SYS32(0x2B4) - h;
    for (i = h; i < end; i++) {
        int v = p[i];
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
    }
    SYS32(omax) = mx;
    SYS32(omin) = mn;
    SYS32(omid) = (mn + mx) / 2;
    SYS32(orng) = mx - mn;
}
