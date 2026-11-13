/*
 * disk_write -- stock 0x40049CE8 (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes through the bounce buffer; wrong drive -> RES_PARERR, unready card -> 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int disk_write(u32 pdrv, const unsigned char *buff, u32 sector, u32 count)
{
    if (pdrv != 0)
        return 4;
    if (g_sd_state != 1)
        return 0;
    memcpy(SD_BOUNCE, buff, g_sector_size * count);
    return mmc_write_blocks(0, sector, count, SD_BOUNCE) ? 1 : 0;
}
