/*
 * dcache_flush_range_se -- stock 0x400688C8 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Every 32-byte line from start to end (inclusive, aligned down).
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "shared/drivers.h"

void dcache_flush_range_se(u32 start, u32 end)
{
    u32 a = start & ~31u, e = end & ~31u;
    for (; a <= e; a += 32)
        dcache_clean_inv_mva(a);
}
