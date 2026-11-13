/*
 * mmc_check_range -- stock 0x40067678 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * start + count - 1 must not exceed the card capacity (sectors); else two messages and error 0xFFFCA9A3.
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

int mmc_check_range(u32 start, u32 count)
{
    u32 end = start + count - 1;
    if (mmc_capacity >= end)
        return 0;
    dbg_printf(STR(0x4008CC4C));
    dbg_printf(STR(0x4008CD70), mmc_capacity);
    return (int)0xFFFCA9A3u;
}
