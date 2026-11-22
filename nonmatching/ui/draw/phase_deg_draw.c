/*
 * phase_deg_draw -- stock 0x40038788 (464 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws a phase: v = (long long)(d * 1000) clamped to -180000..180000, drawn as v / 10 with 2 decimals when |v| <= 9999, v / 100 with 1 decimal up to 99999, v / 1000 with none up to 999999 (number_draw with flag 1 when the low byte of sg is set, else flag 0), then the text "deg" at x + the last drawn width (u16 0x42E68BE4) + 4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "ui/draw.h"

#define TW (*(volatile unsigned short *)0x42E68BE4u)

static void num(u32 x, u32 y, u32 sg, long long v, u32 dec)
{
    if (sg)
        func_4002d82c_flag1(x, y, (u32)v, (u32)((unsigned long long)v >> 32), dec);
    else
        func_4002d82c_flag0(x, y, (u32)v, (u32)((unsigned long long)v >> 32), dec);
}

void phase_deg_draw(u32 x, u32 y, u32 sg, double d)
{
    long long v = (long long)(d * 1000.0);
    sg = (unsigned char)sg;
    if (v < -180000)
        v = -180000;
    else if (v > 180000)
        v = 180000;
    if ((unsigned long long)(v + 9999) < 19999)
        num(x, y, sg, v / 10, 2);
    else if ((unsigned long long)(v + 99999) < 199999)
        num(x, y, sg, v / 100, 1);
    else if ((unsigned long long)(v + 999999) < 1999999)
        num(x, y, sg, v / 1000, 0);
    text_draw((unsigned short)(x + TW + 4), y, "deg");
}
