/*
 * sync_window -- stock 0x40056CF0 (144 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

FRESULT sync_window(FATFS *fs)
{
    FRESULT res = FR_OK;

    if (fs->wflag) {
        if (disk_write(fs->pdrv, fs->win, fs->winsect, 1) == 0) {
            fs->wflag = 0;
            if (fs->winsect - fs->fatbase < fs->fsize) {
                if (fs->n_fats == 2) disk_write(fs->pdrv, fs->win, fs->winsect + fs->fsize, 1);
            }
        } else {
            res = FR_DISK_ERR;
        }
    }
    return res;
}
