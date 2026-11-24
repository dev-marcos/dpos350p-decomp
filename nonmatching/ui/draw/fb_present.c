/*
 * fb_present -- stock 0x4003442C (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copy the back buffer (0x41BFBF68) to the scanned-out framebuffer (0x41FB8FE8), then flush the D-cache.
 */
#include "hw/platform.h"
#include "drivers/sdmmc.h"
#include "stock_asm.h"
#include "ui/draw.h"
#include "util.h"
#include "shared/framebuffer.h"

void fb_present(void)
{
    copy_u16(fb_front, fb_back, FB_PIXELS);
    dcache_clean_inv_all_t();
}
