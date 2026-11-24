/*
 * mmc_base -- stock 0x4006763C (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Controller n register base: 0x01C0F000, 0x01C10000, 0x01C11000; 0 otherwise.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "shared/drivers.h"

u32 mmc_base(u32 n)
{
    if (n == 0) return 0x01C0F000u;
    if (n == 1) return 0x01C10000u;
    if (n == 2) return 0x01C11000u;
    return 0;
}
