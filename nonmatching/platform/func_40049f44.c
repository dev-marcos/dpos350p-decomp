/*
 * smec_timing_reset -- stock 0x40049F44 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Per-index job timing back to 2000 (255 entries).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void smec_timing_reset(void)
{
    volatile u32 *p = g_smec_timing;
    int n = 0xFF;
    do {
        *p++ = 0x7D0;
    } while (--n);
}
