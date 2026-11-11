/*
 * move_window -- stock 0x40056D80 (88 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

FRESULT sync_window(FATFS *fs);

FRESULT move_window(FATFS *fs, DWORD sector)
{
    FRESULT res = FR_OK;

    if (sector != fs->winsect) {
        res = sync_window(fs);
        if (res == FR_OK) {
            if (disk_read(fs->pdrv, fs->win, sector, 1) != 0) {
                sector = 0xFFFFFFFF;
                res = FR_DISK_ERR;
            }
            fs->winsect = sector;
        }
    }
    return res;
}
