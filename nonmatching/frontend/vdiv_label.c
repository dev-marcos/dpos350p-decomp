/*
 * vdiv_label -- stock 0x4004A884 (384 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * V/div text for a channel (CH1 value SYS_INFO+0x50 -> text +0x70, CH2 +0x110 -> +0x130): the uV value divided by 1e9/1e6/1e3 gives up to three digits (no leading zeros) and KV, V or mV. Below 1000 uV the stock code uses the r2/r3 it was called with (passed through here).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "frontend.h"

void vdiv_label(u32 ch, u32 r1, int d, int unit)
{
    long long v;
    char *s;
    int hd, td, od, n = 0, any = 0;
    if (ch == 0) { v = (long long)SYS64(0x50); s = (char *)(0x40544E68u + 0x70); }
    else { v = (long long)SYS64(0x110); s = (char *)(0x40544E68u + 0x130); }
    if (v >= 1000000000LL) { d = (int)__aeabi_ldivmod(v, 1000000000LL); unit = 0; }
    else if (v >= 1000000LL) { d = (int)__aeabi_ldivmod(v, 1000000LL); unit = 1; }
    else if (v >= 1000LL) { d = (int)__aeabi_ldivmod(v, 1000LL); unit = 2; }
    hd = d / 100;
    td = (d - 100 * hd) / 10;
    od = d - 10 * (d / 10);
    if (hd != 0) { s[0] = hd + '0'; n = 1; any = 1; }
    if (any | td) { s[n++] = td + '0'; any = 1; }
    if (any | od) s[n++] = od + '0';
    if (unit == 0) { s[n] = 'K'; s[n + 1] = 'V'; s[n + 2] = 0; }
    else if (unit == 1) { s[n] = 'V'; s[n + 1] = 0; }
    else { s[n] = 'm'; s[n + 1] = 'V'; s[n + 2] = 0; }
}
