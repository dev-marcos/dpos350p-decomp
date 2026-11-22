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

#define FB_PIXELS 0x95B50u                     /* 1022 * 600 */
#define fb_front  ((unsigned short *)0x41FB8FE8u)
#define fb_back   ((unsigned short *)0x41BFBF68u)

void fb_present(void)
{
    copy_u16(fb_front, fb_back, FB_PIXELS);
    dcache_clean_inv_all_t();
}
