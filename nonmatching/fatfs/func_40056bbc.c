/*
 * mem_cmp -- stock 0x40056BBC (36 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

int mem_cmp(const void *dst, const void *src, UINT cnt)
{
    const BYTE *d = (const BYTE *)dst, *s = (const BYTE *)src;
    int r = 0;

    do {
        r = *d++ - *s++;
    } while (--cnt && r == 0);

    return r;
}
