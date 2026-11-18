/*
 * trace_cursor_values -- stock 0x400381E8 (388 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: converts the two cursor positions (words 0x42E22D2C/30) to values. Horizontally: 10^(base + pos / width * scale) for both, their difference; vertically, from the per-x tables at 0x42E20DE8 (gain) / 0x42E21D88 (phase), the two levels in each of two scales and their differences. Results are doubles at 0x42E22D38..0x42E22D78 (block 0x42E180C8 + 0xAC70..0xACB0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

double pow(double x, double y);
#define TD(o) (*(volatile double *)(0x42E180C8u + (o)))
#define TI(o) (*(volatile int *)(0x42E180C8u + (o)))
#define TH(o) (*(volatile unsigned short *)(0x42E180C8u + (o)))

void trace_cursor_values(void)
{
    double sc, w, base, p1, p2, top, bot, h, s5, a1, a2, r, b1, b2;
    int c1, c2;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    sc = TD(0x8D18);
    w = (double)TH(4);
    base = TD(0x8D08);
    p1 = pow(10.0, base + (double)TI(0xAC64) / w * sc);
    p2 = pow(10.0, base + (double)TI(0xAC68) / w * sc);
    top = TD(0x10);
    c1 = TI(0xAC64);
    bot = TD(0x18);
    h = (double)TH(6);
    c2 = TI(0xAC68);
    TD(0xAC70) = p1;
    TD(0xAC78) = p2;
    TD(0xAC80) = __builtin_fabs(p2 - p1);
    r = top - bot;
    a1 = top - r * (double)TH(0x8D20 + 2 * c1) / h;
    a2 = top - r * (double)TH(0x8D20 + 2 * c2) / h;
    s5 = TD(0x20) - TD(0x28);
    b2 = s5 * (double)TH(0x9CC0 + 2 * c2);
    b1 = s5 * (double)TH(0x9CC0 + 2 * c1);
    TD(0xAC88) = a1;
    TD(0xAC90) = a2;
    b2 = b2 / h;
    b1 = b1 / h;
    TD(0xACA8) = TD(0x20) - b2;
    TD(0xAC98) = __builtin_fabs(a2 - a1);
    TD(0xACA0) = TD(0x20) - b1;
    TD(0xACB0) = __builtin_fabs(TD(0xACA8) - TD(0xACA0));
}
