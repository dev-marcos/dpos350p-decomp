/*
 * st_word -- stock 0x40056B04 (16 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

void st_word(BYTE *ptr, WORD val)
{
    *ptr++ = (BYTE)val; val >>= 8;
    *ptr++ = (BYTE)val;
}
