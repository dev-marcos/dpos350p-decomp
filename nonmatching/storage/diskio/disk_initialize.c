/*
 * disk_initialize -- stock 0x40049C38 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Drive 0 only: card init when the SD driver is ready (returns 0, or 2 on failure), else STA_NOINIT.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/sdmmc.h"
#include "storage/diskio.h"

int disk_initialize(u32 pdrv)
{
    if (pdrv != 0)
        return 1;
    if (g_sd_state != 1)
        return 1;
    return mmc_card_init(0) ? 2 : 0;
}
