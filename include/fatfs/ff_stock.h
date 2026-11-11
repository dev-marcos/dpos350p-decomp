/*
 * FatFs R0.13b types and objects as configured in the DPOS350P stock firmware
 * (build 2025-06-19).  Derived from FatFs R0.13b by ChaN (see LICENSE notice below)
 * and from the layout observed in the stock binary (docs/COMPILER.md).
 *
 * Stock configuration (evidence level in brackets):
 *   FF_FS_READONLY 0, FF_USE_MKFS 1                       [DERIVED: f_mkfs 0x4005B1F8]
 *   FF_FS_EXFAT 1, FF_USE_LFN >= 1                        [DERIVED: check_fs, FATFS layout]
 *   FF_CODE_PAGE 437, FF_LFN_UNICODE 0                    [DERIVED: tchar2uni/put_utf pass 0x1B5]
 *   FF_MIN_SS == FF_MAX_SS == 512, FF_FS_RPATH 0,
 *   FF_FS_REENTRANT 0, FF_FS_LOCK 0                       [DERIVED: FATFS offsets, no lock code]
 *
 * ----------------------------------------------------------------------------
 *  FatFs - Generic FAT Filesystem Module  R0.13b
 *  Copyright (C) 2018, ChaN, all right reserved.
 *
 *  FatFs module is an open source software. Redistribution and use of FatFs in
 *  source and binary forms, with or without modification, are permitted provided
 *  that the following condition is met:
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this condition and the following disclaimer.
 *  This software is provided by the copyright holder and contributors "AS IS"
 *  and any warranties related to this software are DISCLAIMED.
 *  The copyright owner or contributors be NOT LIABLE for any damages caused
 *  by use of this software.
 * ----------------------------------------------------------------------------
 */
#ifndef FF_STOCK_H
#define FF_STOCK_H

#include <stddef.h>

typedef unsigned int       UINT;
typedef unsigned char      BYTE;
typedef unsigned short     WORD;
typedef unsigned long      DWORD;
typedef unsigned long long QWORD;
typedef WORD               WCHAR;
typedef char               TCHAR;

#define FF_MIN_SS   512
#define FF_MAX_SS   512
#define SS(fs)      ((UINT)FF_MAX_SS)

typedef enum {
    FR_OK = 0, FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_FILE, FR_NO_PATH,
    FR_INVALID_NAME, FR_DENIED, FR_EXIST, FR_INVALID_OBJECT, FR_WRITE_PROTECTED,
    FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_MKFS_ABORTED, FR_TIMEOUT,
    FR_LOCKED, FR_NOT_ENOUGH_CORE, FR_TOO_MANY_OPEN_FILES, FR_INVALID_PARAMETER
} FRESULT;

typedef BYTE DRESULT_t;     /* diskio DRESULT: RES_OK = 0 */

/* Filesystem object (R0.13b ff.h with the stock options). */
typedef struct {
    BYTE  fs_type;      /* +0x00 */
    BYTE  pdrv;         /* +0x01 */
    BYTE  n_fats;       /* +0x02 */
    BYTE  wflag;        /* +0x03 */
    BYTE  fsi_flag;     /* +0x04 */
    WORD  id;           /* +0x06 */
    WORD  n_rootdir;    /* +0x08 */
    WORD  csize;        /* +0x0A */
    WCHAR *lfnbuf;      /* +0x0C */
    BYTE  *dirbuf;      /* +0x10 */
    DWORD last_clst;    /* +0x14 */
    DWORD free_clst;    /* +0x18 */
    DWORD n_fatent;     /* +0x1C */
    DWORD fsize;        /* +0x20 */
    DWORD volbase;      /* +0x24 */
    DWORD fatbase;      /* +0x28 */
    DWORD dirbase;      /* +0x2C */
    DWORD database;     /* +0x30 */
    DWORD winsect;      /* +0x34 */
    BYTE  win[FF_MAX_SS]; /* +0x38 */
} FATFS;

/* Offsets observed in the stock binary (check_fs/sync_window/move_window). */
_Static_assert(offsetof(FATFS, wflag) == 0x03, "FATFS.wflag");
_Static_assert(offsetof(FATFS, n_fats) == 0x02, "FATFS.n_fats");
_Static_assert(offsetof(FATFS, fsize) == 0x20, "FATFS.fsize");
_Static_assert(offsetof(FATFS, fatbase) == 0x28, "FATFS.fatbase");
_Static_assert(offsetof(FATFS, winsect) == 0x34, "FATFS.winsect");
_Static_assert(offsetof(FATFS, win) == 0x38, "FATFS.win");

#define FS_FAT12 1
#define FS_FAT16 2
#define FS_FAT32 3
#define FS_EXFAT 4

/* Boot sector offsets used by check_fs */
#define BS_JmpBoot      0
#define BS_FilSysType   54
#define BS_FilSysType32 82
#define BS_55AA         510

/* diskio (stock implementations live in the image) */
DRESULT_t disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);   /* 0x40049C78 */
DRESULT_t disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count); /* 0x40049CE8 */

/* ffunicode.c (in the image) */
WCHAR ff_oem2uni(WCHAR oem, WORD cp);   /* 0x40069DF4 */
WCHAR ff_uni2oem(DWORD uni, WORD cp);   /* 0x40069D90 */

#define FF_CODE_PAGE 437

#endif /* FF_STOCK_H */
