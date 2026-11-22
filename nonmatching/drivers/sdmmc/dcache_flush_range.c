/*
 * dcache_flush_range -- stock 0x400688FC (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b dcache_flush_range_se(addr, addr + len).
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

void dcache_flush_range_se(u32 start, u32 end);

void dcache_flush_range(u32 addr, u32 len) { dcache_flush_range_se(addr, addr + len); }
