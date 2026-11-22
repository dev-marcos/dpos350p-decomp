/*
 * disk_ioctl -- stock 0x40049D68 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CTRL_SYNC, GET_SECTOR_COUNT (when ready), GET_SECTOR_SIZE, GET_BLOCK_SIZE (1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/diskio.h"

int disk_ioctl(u32 pdrv, u32 cmd, void *buff)
{
    if (pdrv != 0)
        return 4;
    switch (cmd) {
    case 0:
        return 0;
    case 1:
        if (g_sd_state == 1)
            *(u32 *)buff = g_sd_info[1];
        return 0;
    case 2:
        *(unsigned short *)buff = g_sector_size;
        return 0;
    case 3:
        *(u32 *)buff = 1;
        return 0;
    default:
        return 4;
    }
}
