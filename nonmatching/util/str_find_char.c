/*
 * str_find_char -- stock 0x40056BE8 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Returns the first character of s that is NUL or equal to ch (the character, sign-extended, not a pointer).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "util.h"

int str_find_char(const signed char *s, int ch)
{
    for (;; s++) {
        int c = *s;
        if (c == 0 || c == ch)
            return c;
    }
}
