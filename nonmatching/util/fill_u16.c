/*
 * fill_u16 -- stock 0x400356F4 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Store a halfword n times.
 */
#include "hw/platform.h"
#include "drivers/sdmmc.h"
#include "stock_asm.h"
#include "util.h"
#include "shared/framebuffer.h"

void fill_u16(unsigned short *d, unsigned short v, u32 n)
{
    while (n) { *d++ = v; n--; }
}
