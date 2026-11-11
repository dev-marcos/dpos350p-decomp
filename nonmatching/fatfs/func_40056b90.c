/*
 * mem_cpy -- stock 0x40056B90 (28 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

void mem_cpy(void *dst, const void *src, UINT cnt)
{
    BYTE *d = (BYTE *)dst;
    const BYTE *s = (const BYTE *)src;

    if (cnt != 0) {
        do {
            *d++ = *s++;
        } while (--cnt);
    }
}
