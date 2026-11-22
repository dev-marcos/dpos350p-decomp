/*
 * str_copy -- stock 0x400342B4 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies the NUL-terminated string s to d, terminator included; returns d advanced past the terminator.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "util.h"

unsigned char *str_copy(unsigned char *d, const unsigned char *s)
{
    do
        *d++ = *s;
    while (*s++);
    return d;
}
