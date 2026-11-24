/*
 * trace_cursor_limits -- stock 0x40037944 (284 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: limits the values at 0x42E18114/0x42E18118 by the bounds at 0x42E4289C/0x42E428A0, the two widths 0x42E1811C/0x42E18120 to 10 .. word 0x42E42898 (and to each other in modes 3 and 4 of 0x42E18110), 0x42E18124 to 20..500; then passes them to gen_limits through 0x42E42884/0x42E42888 and copies the result back.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "processing.h"
#include "state/bode_block.h"

#define Q32(o) (*(volatile int *)(0x42E42884u + (o)))

void trace_cursor_limits(void)
{
    int a = BODES(start_freq), b = BODES(stop_freq), v, c, m;
    if (g_touch->serviced)
        return;
    v = Q32(0x18);
    c = BODES(amplitude);
    if (v < c)
        BODES(amplitude) = v;
    else if (c < 0)
        BODES(amplitude) = 0;
    v = Q32(0x1C);
    c = BODES(offset);
    if (v < c)
        BODES(offset) = v;
    else if (c < -v)
        BODES(offset) = -v;
    m = Q32(0x14);
    if (m < a)
        a = m;
    else if (a < 10)
        a = 10;
    if (m < b)
        b = m;
    else if (b < 10)
        b = 10;
    if (BODES(f_048) == 3) {
        if (b <= a)
            a = b;
    } else if (BODES(f_048) == 4) {
        if (a >= b)
            b = a;
    }
    v = BODES(freq_count);
    if (v > 500)
        BODES(freq_count) = 500;
    else if (v < 20)
        BODES(freq_count) = 20;
    Q32(0) = BODES(amplitude);
    Q32(4) = BODES(offset);
    BODES(stop_freq) = b;
    BODES(start_freq) = a;
    gen_limits(a, b, Q32(0), Q32(4));
    BODES(amplitude) = Q32(0);
    BODES(offset) = Q32(4);
}
