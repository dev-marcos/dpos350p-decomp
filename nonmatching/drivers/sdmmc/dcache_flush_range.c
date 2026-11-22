/*
 * dcache_flush_range -- stock 0x400688FC (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b dcache_flush_range_se(addr, addr + len).
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"

#define mmc_capacity (*(u32 *)0x42E68CC0u)

void dcache_flush_range(u32 addr, u32 len) { dcache_flush_range_se(addr, addr + len); }
