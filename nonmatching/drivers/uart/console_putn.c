/*
 * console_putn -- stock 0x40066D28 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Write c n times (n == 0: nothing).
 */
#include "hw/platform.h"
#include "drivers/uart.h"

void console_putn(int c, int n)
{
    if (n)
        do console_putc(c); while (--n);
}
