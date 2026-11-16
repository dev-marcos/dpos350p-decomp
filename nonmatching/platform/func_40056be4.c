/*
 * str_find_char_next -- stock 0x40056BE4 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * str_find_char(s + 1, ch) (falls into 0x40056BE8).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int str_find_char(const signed char *s, int ch);

int str_find_char_next(const signed char *s, int ch)
{
    return str_find_char(s + 1, ch);
}
