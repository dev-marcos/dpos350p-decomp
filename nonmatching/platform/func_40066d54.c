/*
 * console_put_reversed -- stock 0x40066D54 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Write buf[n-1] .. buf[0].
 */
#include "hw/platform.h"

void console_put_reversed(const char *buf, int n)
{
    const char *p;
    if (n < 1) return;
    p = buf + n - 1;
    do console_putc((signed char)*p--); while (--n);
}
