/*
 * mmc_clk_disable -- stock 0x40067918 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CLKCR card clock (bit 16), then an update-clock command; -1 when that fails.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "shared/drivers.h"

int mmc_clk_disable(u32 n)
{
    u32 r = mmc_base(n) + 4;
    writel_a(r, readl_a(r) & ~0x10000u);
    return mmc_update_clk(n) ? -1 : 0;
}
