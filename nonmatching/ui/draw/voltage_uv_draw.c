/*
 * voltage_uv_draw -- stock 0x40038974 (636 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws a voltage given in microvolts: v = (long long)d; |v| <= 999 as v "uV", up to 999999 as v / 1000 "mV" (no decimals), up to 9999999 as v / 10000 with 2 decimals "V", up to 99999999 as v / 100000 with 1 decimal "V"; larger values draw nothing. Sign flag and unit position as in phase_deg_draw.
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

void voltage_uv_draw(u32 x, u32 y, u32 sg, double d)
{
    long long v = (long long)d;
    const char *u;
    sg = (unsigned char)sg;
    if ((unsigned long long)(v + 999) < 1999) {
        num(x, y, sg, v, 0);
        u = "uV";
    } else if ((unsigned long long)(v + 999999) < 1999999) {
        num(x, y, sg, v / 1000, 0);
        u = "mV";
    } else if ((unsigned long long)(v + 9999999) < 19999999) {
        num(x, y, sg, v / 10000, 2);
        u = "V";
    } else if ((unsigned long long)(v + 99999999) < 199999999) {
        num(x, y, sg, v / 100000, 1);
        u = "V";
    } else
        return;
    text_draw((unsigned short)(x + TW + 4), y, u);
}
