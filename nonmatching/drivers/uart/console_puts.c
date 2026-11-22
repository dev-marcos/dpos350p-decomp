/*
 * console_puts -- stock 0x40066D04 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Write a NUL-terminated string.
 */
#include "hw/platform.h"
#include "drivers/uart.h"

void console_puts(const char *s)
{
    while (*s)
        console_putc((signed char)*s++);
}
