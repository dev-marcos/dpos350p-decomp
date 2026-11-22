/*
 * mmc_base -- stock 0x4006763C (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Controller n register base: 0x01C0F000, 0x01C10000, 0x01C11000; 0 otherwise.
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

u32 mmc_base(u32 n)
{
    if (n == 0) return 0x01C0F000u;
    if (n == 1) return 0x01C10000u;
    if (n == 2) return 0x01C11000u;
    return 0;
}
