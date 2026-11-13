/*
 * mmc_set_bus_width -- stock 0x40067CFC (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CTYPE (+0x0C) = width code; returns 0.
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

int mmc_set_bus_width(u32 n, u32 ctype)
{
    writel_a(mmc_base(n) + 0x0C, ctype);
    return 0;
}
