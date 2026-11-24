/*
 * fill_u32 -- stock 0x4003570C (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Store a word n times.
 */
#include "hw/platform.h"
#include "drivers/sdmmc.h"
#include "stock_asm.h"
#include "util.h"
#include "shared/framebuffer.h"

void fill_u32(u32 *d, u32 v, u32 n)
{
    while (n) { *d++ = v; n--; }
}
