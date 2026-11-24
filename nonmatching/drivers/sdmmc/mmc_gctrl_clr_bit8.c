/*
 * mmc_gctrl_clr_bit8 -- stock 0x40067958 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GCTRL (+0x00) &= ~bit 8; returns 0.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "shared/drivers.h"

int mmc_gctrl_clr_bit8(u32 n)
{
    u32 r = mmc_base(n);
    writel_a(r, readl_a(r) & ~0x100u);
    return 0;
}
