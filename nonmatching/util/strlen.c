/*
 * strlen -- stock 0x4006A6A8 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Length of a NUL-terminated string (the stock code scans a word at a time once aligned). (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "util.h"

u32 strlen(const char *str)
{
    const volatile char *p = str;
    while (*p)
        p++;
    return (u32)(p - str);
}
