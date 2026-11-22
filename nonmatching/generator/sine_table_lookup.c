/*
 * sine_table_lookup -- stock 0x4005D220 (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Table 0x42D564B4 of 20001 words: T[x mod 20001] for x >= 0, -T[(-x) mod 20001] for x < 0 (signed remainder, as the stock multiply-high division).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"

#define SINT ((volatile int *)0x42D564B4u)

int sine_table_lookup(int x)
{
    if (x < 0) {
        int n = (int)(0u - (u32)x);
        return -SINT[n % 20001];
    }
    return SINT[x % 20001];
}
