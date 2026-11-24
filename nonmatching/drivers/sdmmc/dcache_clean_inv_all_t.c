/*
 * dcache_clean_inv_all_t -- stock 0x4003E6F4 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b dcache_clean_inv_all.
 */
#include "hw/platform.h"
#include "drivers/sdmmc.h"
#include "stock_asm.h"
#include "util.h"
#include "shared/framebuffer.h"

void dcache_clean_inv_all_t(void) { dcache_clean_inv_all(); }
