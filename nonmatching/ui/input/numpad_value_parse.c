/*
 * numpad_value_parse -- stock 0x4005FC84 (520 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Numpad text to value: the text at 0x42E68A8B (an optional '-', then up to 20 characters; digits until the first other character, a '.' switches to decimals) becomes a double; by the numpad mode byte 0x42E68A89 and the unit key (0xE, 0xF, 0x10 of numpad_key) it is scaled by 1, 1000, 1e6 or 10, or the old value 0x42E68AA8 is kept. When the text is not empty the double goes to the target *0x42E68AB0, the value clamped to -0x7FFFFFFF..0x7FFFFFFF as s64 to *0x42E68AB4 and its low word to *0x42E68AB8, and the callback 0x42E68AD8 runs if flag 0x42E68AD4 is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "ui/input.h"
#include "state/entry_pad.h"

#define NPS8(o) (*(volatile signed char *)(0x42E68A88u + (o)))

void numpad_value_parse(u32 unit)
{
    double v = NUMPAD(value), acc = 0.0, x;
    unsigned long long scale = 1, f = 1;
    int neg = NPS8(3) == '-', i;
    u32 m;
    for (i = neg; i < 20; i++) {
        u32 ch = (unsigned short)NPS8(3 + i);
        if (ch == '.') {
            f = 10;
            continue;
        }
        if (ch - 0x30 >= 10)
            break;
        scale *= f;
        if (neg)
            acc = acc * 10.0 - (double)(unsigned short)(ch - 0x30);
        else
            acc = acc * 10.0 + (double)(unsigned short)(ch - 0x30);
    }
    x = acc / (double)scale;
    m = NUMPAD(mode);
    if (unit == 0xE && m >= 1 && m <= 5)
        v = x;
    else if (m == 1 && unit == 0xF)
        v = x * 1000.0;
    else if (m == 1 && unit == 0x10)
        v = x * 1000000.0;
    else if (m == 4 && unit == 0xF)
        v = x * 1000.0;
    else if (m == 6 && unit == 0xE)
        v = x * 10.0;
    NUMPAD(value) = v;
    if (NPS8(3)) {
        long long q = (long long)v;
        if (q > 0x7FFFFFFFLL)
            q = 0x7FFFFFFFLL;
        else if (q < -0x7FFFFFFFLL)
            q = -0x7FFFFFFFLL;
        *(volatile double *)NUMPAD(target_0) = v;
        *(volatile long long *)NUMPAD(target_1) = q;
        *(volatile u32 *)NUMPAD(target_2) = (u32)q;
        if (NUMPAD(f_04c))
            ((void (*)(void))NUMPAD(f_050))();
        v = NUMPAD(value);
    }
    NUMPAD(value) = v;
}
