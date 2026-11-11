/*
 * ld_qword -- stock 0x40056AA0 (100 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

QWORD ld_qword(const BYTE *ptr)
{
    QWORD rv;

    rv = ptr[7];
    rv = rv << 8 | ptr[6];
    rv = rv << 8 | ptr[5];
    rv = rv << 8 | ptr[4];
    rv = rv << 8 | ptr[3];
    rv = rv << 8 | ptr[2];
    rv = rv << 8 | ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}
