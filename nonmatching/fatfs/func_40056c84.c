/*
 * put_utf -- stock 0x40056C84 (108 bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (see docs/PROGRESS.md)
 * Source: FatFs R0.13b ff.c by ChaN, stock configuration (include/fatfs/ff_stock.h).
 * FatFs licence: see the notice in include/fatfs/ff_stock.h (retained as required).
 */
#include "fatfs/ff_stock.h"

/* ANSI/OEM output (FF_LFN_UNICODE 0) */
BYTE put_utf(DWORD chr, TCHAR *buf, UINT szb)
{
    WCHAR wc;

    wc = ff_uni2oem(chr, FF_CODE_PAGE);
    if (wc >= 0x100) {
        if (szb < 2) return 0;
        *buf++ = (char)(wc >> 8);
        *buf++ = (TCHAR)wc;
        return 2;
    }
    if (wc == 0 || szb < 1) return 0;
    *buf++ = (TCHAR)wc;
    return 1;
}
