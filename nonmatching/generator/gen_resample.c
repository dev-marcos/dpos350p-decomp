/*
 * gen_resample -- stock 0x4004D49C (304 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Resamples one period of the table: period = length / count (descriptor G+0xFAE); step = (u64)(period x 1.02) x SYS_INFO+0x554 x m / total with m = total / period + 1; decimates by m, resamples into the work buffer 0x41648008 with the nearest-neighbour mapper, copies back, repeats the end samples and clamps to G+0xFB0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void gen_resample(void)
{
    u32 per = (int)G16(0xFAE) / (int)G16(0xFAE + 6);
    u32 tot = G16(0x2EF8);
    u32 m = tot / per + 1;
    unsigned long long x = D2ULZ((double)per * 1.02);
    u32 step;
    unsigned short *w = (unsigned short *)(0x416431E8u + 0x4E20);
    u32 i;
    x = x * SYS32(0x554);
    x = x * m;
    step = (u32)__aeabi_uldivmod(x, tot);
    gen_decimate(m & 0xFF);
    {
        u32 q = tot / m;
        copy_u16(w - (q >> 1), GEN_BUF, q);
    }
    simple_integer_index_mapping_nearest_neighbour(0x416431E8u, 10000, per & 0xFFFF, step, 0, 0);
    copy_u16(GEN_BUF, w - (per >> 1), per);
    GEN_BUF[0] = GEN_BUF[1];
    GEN_BUF[per - 1] = GEN_BUF[per - 2];
    for (i = 0; i < per; i++) {
        unsigned short lim = G16(0xFB0), v = GEN_BUF[i];
        GEN_BUF[i] = lim > v ? v : lim;
    }
}
