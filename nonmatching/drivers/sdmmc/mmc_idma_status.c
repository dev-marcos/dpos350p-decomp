/*
 * mmc_idma_status -- stock 0x40067D1C (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * IDST (+0x88) TX/RX interrupt bits; acknowledged (write 1) when set; returns them.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"

#define mmc_capacity (*(u32 *)0x42E68CC0u)

int mmc_idma_status(u32 n)
{
    u32 r = mmc_base(n) + 0x88, v = readl_a(r);
    int st = 0;
    if (v & 1) st = 1;
    if (v & 2) st |= 2;
    if (st > 0)
        writel_a(r, (u32)st | readl_a(r));
    return st;
}
