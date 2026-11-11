/*
 * dbc_2nd -- stock 0x40056C08 (8 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

int dbc_2nd(BYTE c)
{
    if (c != 0) return 0;   /* SBCS fixed code page: always false */
    return 0;
}
