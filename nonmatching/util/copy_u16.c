/*
 * copy_u16 -- stock 0x40035728 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copy n halfwords.
 */
#include "hw/platform.h"
#include "drivers/sdmmc.h"
#include "stock_asm.h"
#include "util.h"
#include "shared/framebuffer.h"

void copy_u16(unsigned short *d, const unsigned short *s, u32 n)
{
    while (n) { *d++ = *s++; n--; }
}
