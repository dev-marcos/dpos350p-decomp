/*
 * mmc_gctrl_clr_bit8 -- stock 0x40067958 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GCTRL (+0x00) &= ~bit 8; returns 0.
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

int mmc_gctrl_clr_bit8(u32 n)
{
    u32 r = mmc_base(n);
    writel_a(r, readl_a(r) & ~0x100u);
    return 0;
}
