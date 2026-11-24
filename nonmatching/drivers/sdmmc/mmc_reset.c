/*
 * mmc_reset -- stock 0x40067A04 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GCTRL |= 7 (soft, FIFO, DMA reset), wait 1 ms; returns 0.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "shared/drivers.h"

int mmc_reset(u32 n)
{
    u32 r = mmc_base(n);
    writel_a(r, readl_a(r) | 7);
    mmc_delay_us(1000);
    return 0;
}
