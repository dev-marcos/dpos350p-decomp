/*
 * console_format_conv -- stock 0x40066ED0 (232 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * One conversion after '%': flags '0' (pad) and '-' (left), width digits, then c s d i u x X o '%'; any other character prints the argument in base 3. Returns the pointer after the conversion. The cases are the fragments 0x40066FB8..0x40067098.
 */
#include "hw/platform.h"
#include "drivers/uart.h"
#include "util.h"

const char *console_format_conv(const char *p, int arg)
{
    int width = 0, left = 0, base = 0, pad = ' ', c;
    unsigned int n;
    if (*p == '0') pad = '0';
    for (;;) {
        c = (signed char)*p++;
        if (c == 0) break;
        if ((unsigned)(c - '0') < 10) { width = width * 10 + c - '0'; continue; }
        if ((unsigned)c > 'x') { base = 3; break; }
        switch (c) {
        case '-': left = 1; continue;
        case '%': console_putc('%'); return p;
        case 'c':
            if (left) console_putc(arg & 0x7F);
            if (width > 0) console_putn(pad, width - 1);
            if (left) return p;
            console_putc(arg & 0x7F);
            return p;
        case 's':
            if (left) console_puts((const char *)arg);
            n = strlen((const char *)arg);
            if (n < (unsigned)width) console_putn(pad, width - (int)strlen((const char *)arg));
            if (left) return p;
            console_puts((const char *)arg);
            return p;
        case 'd': case 'i': base = -10; break;
        case 'u': base = 10; break;
        case 'X': case 'x': base = 16; break;
        case 'o': base = 8; break;
        default: base = 3; break;
        }
        break;
    }
    if (left) width = -width;
    console_print_num(arg, base, width, pad);
    return p;
}
