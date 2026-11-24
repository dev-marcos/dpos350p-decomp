/*
 * mmc_clk_enable -- stock 0x400678D8 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CLKCR card clock (bit 16), then an update-clock command; -1 when that fails.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "shared/drivers.h"

int mmc_clk_enable(u32 n)
{
    u32 r = mmc_base(n) + 4;
    writel_a(r, readl_a(r) | 0x10000);
    return mmc_update_clk(n) ? -1 : 0;
}
