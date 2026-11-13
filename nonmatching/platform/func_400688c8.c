/*
 * dcache_flush_range_se -- stock 0x400688C8 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Every 32-byte line from start to end (inclusive, aligned down).
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

void dcache_clean_inv_mva(u32 addr);

void dcache_flush_range_se(u32 start, u32 end)
{
    u32 a = start & ~31u, e = end & ~31u;
    for (; a <= e; a += 32)
        dcache_clean_inv_mva(a);
}
