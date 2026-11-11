/*
 * ld_word -- stock 0x40056A70 (16 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

WORD ld_word(const BYTE *ptr)
{
    WORD rv;

    rv = ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}
