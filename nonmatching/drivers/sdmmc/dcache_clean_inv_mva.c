/*
 * dcache_clean_inv_mva -- stock 0x400688BC (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clean+invalidate the D-cache line and invalidate the I-cache line of an address (CP15 c7).
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

void dcache_clean_inv_mva(u32 addr)
{
    __asm__ volatile ("mcr p15, 0, %0, c7, c14, 1" : : "r" (addr) : "memory");
    __asm__ volatile ("mcr p15, 0, %0, c7, c5, 1" : : "r" (addr) : "memory");
}
