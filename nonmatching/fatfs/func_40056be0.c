/*
 * chk_chr -- stock 0x40056BE0 (32 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

/* stock: signed char load (ldrsb) => plain char is signed in this build (OPEN: armcc default is unsigned) */
int chk_chr(const signed char *str, int chr)
{
    while (*str && *str != chr) str++;
    return *str;
}
