/*
 * gen_resample2 -- stock 0x4004D730 (280 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * gen_resample for the second table: period from G+0x2EF6, then each sample capped at 2 x G+0x2F58.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "processing.h"
#include "util.h"
#include "sys_info.h"

void gen_resample2(void)
{
    u32 per = G16(0x2EF6);
    u32 tot = G16(0x2EF8);
    u32 m = tot / per + 1;
    unsigned long long x = D2ULZ((double)per * 1.02);
    u32 step, i;
    unsigned short *w = (unsigned short *)(0x416431E8u + 0x4E20);
    x = x * SI(f_554);
    x = x * m;
    step = (u32)__aeabi_uldivmod(x, tot);
    gen_decimate2(m & 0xFF);
    {
        u32 q = tot / m;
        copy_u16(w - (q >> 1), GEN_BUF2, q);
    }
    simple_integer_index_mapping_nearest_neighbour(0x416431E8u, 10000, per, step, 0, 0);
    copy_u16(GEN_BUF2, w - (per >> 1), per);
    for (i = 0; i < per; i++) {
        u32 lim = G32(0x2F58);
        if (GEN_BUF2[i] > (lim << 1))
            GEN_BUF2[i] = lim << 1;
    }
}
