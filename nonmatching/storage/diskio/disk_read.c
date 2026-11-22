/*
 * disk_read -- stock 0x40049C78 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads through the bounce buffer 0x427EB634; copies it to the caller even when the read failed. A wrong drive or an unready card returns 0 (stock behaviour).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/sdmmc.h"
#include "storage/diskio.h"
#include "util.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
int mmc_read_blocks_raw(int, u32, u32, void *) __asm__("mmc_read_blocks");

int disk_read(u32 pdrv, unsigned char *buff, u32 sector, u32 count)
{
    int res = 0;
    if (pdrv == 0 && g_sd_state == 1) {
        if (mmc_read_blocks_raw(0, sector, count, SD_BOUNCE) != 0)
            res = 1;
        memcpy(buff, SD_BOUNCE, g_sector_size * count);
    }
    return res;
}
