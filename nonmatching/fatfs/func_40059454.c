/*
 * check_fs -- stock 0x40059454 (196 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

FRESULT move_window(FATFS *fs, DWORD sector);
WORD ld_word(const BYTE *ptr);
int mem_cmp(const void *dst, const void *src, UINT cnt);

/* 0:FAT, 1:exFAT, 2:valid BS but not FAT, 3:not a BS, 4:disk error */
BYTE check_fs(FATFS *fs, DWORD sect)
{
    fs->wflag = 0; fs->winsect = 0xFFFFFFFF;
    if (move_window(fs, sect) != FR_OK) return 4;
    if (ld_word(fs->win + BS_55AA) != 0xAA55) return 3;
    if (!mem_cmp(fs->win + BS_JmpBoot, "\xEB\x76\x90" "EXFAT   ", 11)) return 1;
    if (fs->win[BS_JmpBoot] == 0xE9 || fs->win[BS_JmpBoot] == 0xEB || fs->win[BS_JmpBoot] == 0xE8) {
        if (!mem_cmp(fs->win + BS_FilSysType, "FAT", 3)) return 0;
        if (!mem_cmp(fs->win + BS_FilSysType32, "FAT32", 5)) return 0;
    }
    return 2;
}
