/*
 * spi0_clk_init -- stock 0x40042E8C (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CCU SPI0_CLK (0x01C200A0): gate off, source PLL_PERIPH0 with divider 3, gate on; returns PLL_PERIPH0/3 for controller 0, else 0.
 */
#include "hw/drivers.h"

s32 periph0_freq(void);
u32 readl_f(u32 addr);
void writel_f(u32 addr, u32 v);

s32 spi0_clk_init(int ctrl)
{
    if (ctrl != 0) return 0;
    s32 f = periph0_freq();
    writel_f(0x01C200A0u, readl_f(0x01C200A0u) & ~0x80000000u);
    writel_f(0x01C200A0u, 0x01000002u);
    writel_f(0x01C200A0u, readl_f(0x01C200A0u) | 0x80000000u);
    s32 q = (s32)(((s64)f * 0x55555556) >> 32);
    return q + (s32)((u32)q >> 31);
}
