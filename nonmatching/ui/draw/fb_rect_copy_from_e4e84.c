/*
 * fb_rect_copy_from_e4e84 -- stock 0x4003436C (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies the rectangle x0 <= x < x1, y0 <= y < y1 (unsigned) of 16-bit pixels, stride 1022, from the buffer 0x420E4E84 to 0x41FB8FE8, then dcache_clean_inv_all_t (with the same r0..r3).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/sdmmc.h"
#include "ui/draw.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void dcache_clean_inv_all_t_raw(u32 a, u32 b, u32 c, u32 d) __asm__("dcache_clean_inv_all_t");

void fb_rect_copy_from_e4e84(u32 x0, u32 y0, u32 x1, u32 y1)
{
    u32 y, n;
    for (y = y0; y < y1; y++) {
        if (x0 < x1) {
            volatile unsigned short *s = (volatile unsigned short *)(0x420E4E84u + (x0 + y * 1022u) * 2u);
            volatile unsigned short *d = (volatile unsigned short *)(0x41FB8FE8u + (x0 + y * 1022u) * 2u);
            for (n = x1 - x0; n; n--)
                *d++ = *s++;
        }
    }
    dcache_clean_inv_all_t_raw(x0, y0, x1, y1);
}
