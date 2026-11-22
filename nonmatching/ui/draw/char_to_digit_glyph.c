/*
 * char_to_digit_glyph -- stock 0x4002E7B0 (200 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Maps '0'..'9' to 0..9, '.' to 10, '+' to 11 and '-' to 12; any other character to 0xFFFF.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

unsigned short char_to_digit_glyph(u32 c)
{
    switch (c) {
    case '+': return 11;
    case '-': return 12;
    case '.': return 10;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        return c - '0';
    default: return 0xFFFF;
    }
}
