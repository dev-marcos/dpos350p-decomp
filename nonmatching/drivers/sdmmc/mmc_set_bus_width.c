/*
 * mmc_set_bus_width -- stock 0x40067CFC (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CTYPE (+0x0C) = width code; returns 0.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "shared/drivers.h"

int mmc_set_bus_width(u32 n, u32 ctype)
{
    writel_a(mmc_base(n) + 0x0C, ctype);
    return 0;
}
