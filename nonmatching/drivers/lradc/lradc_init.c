/*
 * lradc_init -- stock 0x40068E94 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * LRADC control 0x01C22800: clear bits 12-13, set bit 13, clear bits 4-5, rewrite, then set bit 0 (enable), each a read-modify-write.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

u32 readl_c(u32 addr);
void writel_c(u32 addr, u32 v);

void lradc_init(void)
{
    const u32 r = 0x01C22800u;
    writel_c(r, readl_c(r) & ~0x3000u);
    writel_c(r, readl_c(r) | 0x2000u);
    writel_c(r, readl_c(r) & ~0x30u);
    writel_c(r, readl_c(r));
    writel_c(r, readl_c(r) | 1u);
}
