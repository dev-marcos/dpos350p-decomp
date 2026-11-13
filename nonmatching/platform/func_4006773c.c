/*
 * mmc_update_clk -- stock 0x4006773C (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CMD = 0x80202000 (update clock only, wait previous data), wait for the start bit to clear (200000 x 1 us).
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

int mmc_update_clk(u32 n)
{
    u32 base = mmc_base(n), t = 200000;
    writel_a(base + 0x18, 0x80202000u);
    while (readl_a(base + 0x18) & 0x80000000u) {
        if (t-- == 0) {
            dbg_printf(STR(0x4008F83C));
            return -1;
        }
        mmc_delay_us(1);
    }
    return 0;
}
