/*
 * tchar2uni -- stock 0x40056C10 (116 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

int dbc_1st(BYTE c);
int dbc_2nd(BYTE c);

/* ANSI/OEM input (FF_LFN_UNICODE 0) */
DWORD tchar2uni(const TCHAR **str)
{
    DWORD uc;
    const TCHAR *p = *str;
    BYTE b;
    WCHAR wc;

    wc = (BYTE)*p++;
    if (dbc_1st((BYTE)wc)) {
        b = (BYTE)*p++;
        if (!dbc_2nd(b)) return 0xFFFFFFFF;
        wc = (wc << 8) + b;
    }
    if (wc != 0) {
        wc = ff_oem2uni(wc, FF_CODE_PAGE);
        if (wc == 0) return 0xFFFFFFFF;
    }
    uc = wc;
    *str = p;
    return uc;
}
