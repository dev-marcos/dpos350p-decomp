/*
 * mmc_clk_lowpower_on -- stock 0x400678B0 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CLKCR (+0x04) |= bit 17; returns 0.
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

int mmc_clk_lowpower_on(u32 n)
{
    u32 r = mmc_base(n) + 4;
    writel_a(r, readl_a(r) | 0x20000);
    return 0;
}
