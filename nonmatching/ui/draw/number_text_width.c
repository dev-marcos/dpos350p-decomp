/*
 * number_text_width -- stock 0x4002D2D4 (1024 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Width in pixels (text_width) of the signed 64-bit value v printed with dec decimals (0..15) and the sign mode (0: '-' only; 1: '-' or '+'; other: none). Digits are taken with 64-bit divisions by powers of ten (__aeabi_ldivmod).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

/* Formats v with dec decimals into buf: 16 digits are taken (digit 15 is v / 10^15, so it can
   exceed 9), leading zeros above the units digit are dropped. mode 0: '-' when negative; mode 1:
   '-' or '+'; any other mode (-1 included): no sign. dec must be 0..15 (the stock code has room
   for 16 digits only). */
static void number_format(char *buf, long long v, int dec, int mode)
{
    int d[16], neg = 0, top = 0, n, k, p = 0;
    long long p10;
    if (v < 0) {
        v = (long long)(0 - (unsigned long long)v);
        neg = 1;
    }
    d[0] = (int)(v % 10);
    for (k = 1, p10 = 10; k < 15; k++, p10 *= 10)
        d[k] = (int)((v % (p10 * 10)) / p10);
    d[15] = (int)(v / 1000000000000000LL);
    for (k = 15; k >= 0; k--)
        if (d[k]) {
            top = k;
            break;
        }
    n = dec < top ? top : dec;
    for (k = 0; k <= n; k++)
        d[k] += '0';
    if (mode == 0) {
        if (neg)
            buf[p++] = '-';
    } else if (mode == 1) {
        buf[p++] = neg ? '-' : '+';
    }
    for (k = n; k >= dec; k--)
        buf[p++] = (char)d[k];
    if (dec)
        buf[p++] = '.';
    for (k = dec - 1; k >= 0; k--)
        buf[p++] = (char)d[k];
    buf[p] = 0;
}

u32 number_text_width(long long v, u32 dec, u32 mode)
{
    char buf[20];
    number_format(buf, v, (int)dec, (int)mode);
    return text_width(buf);
}
