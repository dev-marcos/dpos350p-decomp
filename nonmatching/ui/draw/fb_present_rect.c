/*
 * fb_present_rect -- stock 0x400343C4 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies rows y0..y1-1, pixels x0..x1 (inclusive), of the back buffer into the scanned-out framebuffer, then flushes the D-cache.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
#define FB_W     1022u
#define fb_front ((unsigned short *)0x41FB8FE8u)
#define fb_back  ((unsigned short *)0x41BFBF68u)
#define fb_off   ((unsigned short *)0x4233CBBCu)

void dcache_clean_inv_all_t(void);

void fb_present_rect(u32 x0, u32 y0, u32 x1, u32 y1)
{
    u32 n = x1 - x0 + 1, rows;
    unsigned short *s, *d;
    if (y0 < y1) {
        s = fb_back + y0 * FB_W + x0;
        d = fb_front + y0 * FB_W + x0;
        for (rows = y1 - y0; rows; rows--) {
            copy_u16(d, s, n);
            d += FB_W;
            s += FB_W;
        }
    }
    dcache_clean_inv_all_t();
}
