/*
 * percent_at_least -- stock 0x4004C794 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 1 when max(v, 0) x 100 / total >= min_pct or total <= 0, else 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"

u32 percent_at_least(int v, int total, int min_pct)
{
    if (v < 0)
        v = 0;
    if (total <= 0)
        return 1;
    if ((v * 100) / total < min_pct)
        return 0;
    return 1;
}
