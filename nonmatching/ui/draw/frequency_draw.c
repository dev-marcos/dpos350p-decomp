/*
 * frequency_draw -- stock 0x40038E40 (1016 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Frequency v = (long long)d in Hz: below 1000 as v "Hz"; up to 10^4 / 10^5 / 10^6 as v / 10, 100, 1000 "KHz" with 2 / 1 / 0 decimals; then likewise "MHz" up to 10^9 and v / 10^7 "GHz" with 2 decimals below 10^10; larger values draw nothing. Except below 1000 the quotient q is bumped by one when q % 10 >= 5 (stock rounding, it looks at the kept digit). With measure = 0 it draws the number (flag 0) and the unit at x + the drawn width + 4 and returns 0; with measure set it draws nothing and returns number width + unit width + 4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "ui/draw.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
u32 number_text_width_mode0_raw(u32 lo, u32 hi, u32 dec) __asm__("number_text_width_mode0");

#define TW (*(volatile unsigned short *)0x42E68BE4u)

u32 frequency_draw(u32 x, u32 y, u32 measure, double d)
{
    long long v = (long long)d, q;
    const char *u;
    u32 dec;
    measure = (unsigned char)measure;
    if (v < 1000) {
        q = v;
        dec = 0;
        u = "Hz";
    } else {
        if (v < 10000) {
            q = v / 10;
            dec = 2;
            u = "KHz";
        } else if (v < 100000) {
            q = v / 100;
            dec = 1;
            u = "KHz";
        } else if (v < 1000000) {
            q = v / 1000;
            dec = 0;
            u = "KHz";
        } else if (v < 10000000) {
            q = v / 10000;
            dec = 2;
            u = "MHz";
        } else if (v < 100000000) {
            q = v / 100000;
            dec = 1;
            u = "MHz";
        } else if (v < 1000000000) {
            q = v / 1000000;
            dec = 0;
            u = "MHz";
        } else if (v < 10000000000LL) {
            q = v / 10000000;
            dec = 2;
            u = "GHz";
        } else
            return 0;
        if (q % 10 >= 5)
            q++;
    }
    if (measure)
        return number_text_width_mode0_raw((u32)q, (u32)((unsigned long long)q >> 32), dec) + text_width(u) + 4;
    number_draw_mode0(x, y, (u32)q, (u32)((unsigned long long)q >> 32), dec);
    text_draw((unsigned short)(x + TW + 4), y, u);
    return 0;
}
