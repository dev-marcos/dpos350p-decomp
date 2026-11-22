/*
 * trace_cursor_limits -- stock 0x40037944 (284 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: limits the values at 0x42E18114/0x42E18118 by the bounds at 0x42E4289C/0x42E428A0, the two widths 0x42E1811C/0x42E18120 to 10 .. word 0x42E42898 (and to each other in modes 3 and 4 of 0x42E18110), 0x42E18124 to 20..500; then passes them to gen_limits through 0x42E42884/0x42E42888 and copies the result back.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "processing.h"

#define TC32(o) (*(volatile int *)(0x42E180C8u + (o)))
#define Q32(o) (*(volatile int *)(0x42E42884u + (o)))

void trace_cursor_limits(void)
{
    int a = TC32(0x54), b = TC32(0x58), v, c, m;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    v = Q32(0x18);
    c = TC32(0x4C);
    if (v < c)
        TC32(0x4C) = v;
    else if (c < 0)
        TC32(0x4C) = 0;
    v = Q32(0x1C);
    c = TC32(0x50);
    if (v < c)
        TC32(0x50) = v;
    else if (c < -v)
        TC32(0x50) = -v;
    m = Q32(0x14);
    if (m < a)
        a = m;
    else if (a < 10)
        a = 10;
    if (m < b)
        b = m;
    else if (b < 10)
        b = 10;
    if (TC32(0x48) == 3) {
        if (b <= a)
            a = b;
    } else if (TC32(0x48) == 4) {
        if (a >= b)
            b = a;
    }
    v = TC32(0x5C);
    if (v > 500)
        TC32(0x5C) = 500;
    else if (v < 20)
        TC32(0x5C) = 20;
    Q32(0) = TC32(0x4C);
    Q32(4) = TC32(0x50);
    TC32(0x58) = b;
    TC32(0x54) = a;
    gen_limits(a, b, Q32(0), Q32(4));
    TC32(0x4C) = Q32(0);
    TC32(0x50) = Q32(4);
}
