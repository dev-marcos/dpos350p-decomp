/*
 * screenshot_thumb_make -- stock 0x40034E94 (472 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Builds the preview of a screenshot: copies the scanned-out framebuffer 0x41FB8FE8 (1022 x 600 RGB565) to 0x42210D20, then box-filters it in steps of 5.035 x 4.348 pixels (6 x 5 samples per output pixel, coordinates in 1/1000 pixel) into a 203 x 138 picture. Header at SYS+0x109684: u16 width (counted per row) and u16 height; the pixels follow at SYS+0x109688. Each channel is the sum of its samples x 1e6 / 5035 / 4348, clamped to 31 / 63 / 31.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "util.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define TW (SYS + 0x109684u)     /* u16 width, u16 height, then the RGB565 pixels */

/* (1e6 * sum) / a / b with the stock reciprocal multiplications */
static u32 scale(u32 sum, u32 m1, u32 m2, int s2)
{
    u32 t = (u32)(((unsigned long long)(sum * 1000000u) * m1) >> 32) >> 12;
    return (u32)(((unsigned long long)t * m2) >> 32) >> s2;
}

static unsigned short pack(u32 r, u32 g, u32 b)
{
    if (r > 0x1F)
        r = 0x1F;
    if (g > 0x3F)
        g = 0x3F;
    if (b > 0x1F)
        b = 0x1F;
    return (unsigned short)(r << 11 | g << 5 | b);
}

void screenshot_thumb_make(void)
{
    volatile unsigned short *fb = (volatile unsigned short *)0x42210D20u;
    volatile unsigned short *dst = (volatile unsigned short *)(SYS + 0x109688u);
    int x, y = 0, i, j, k = 0;
    copy_u16((unsigned short *)0x42210D20u, (const unsigned short *)0x41FB8FE8u, 0x95B50);
    W32(TW) = 0;
    do {
        H16(TW) = 0;
        H16(TW + 2) = (unsigned short)(H16(TW + 2) + 1);
        x = 0;
        do {
            u32 r = 0, g = 0, b = 0;
            for (i = 0; i < 5; i++) {
                u32 row = 1022u * (u32)((y + 1000 * i) / 1000);
                for (j = 0; j < 6; j++) {
                    u32 p = fb[(u32)((x + 1000 * j) / 1000) + row];
                    r += p >> 11 & 0x1F;
                    g += p >> 5 & 0x3F;
                    b += p & 0x1F;
                }
            }
            dst[k++] = pack(scale(r, 0xD041E4DAu, 0xF129AF75u, 12), scale(g, 0xD041E4DAu, 0xF129AF75u, 12),
                            scale(b, 0xD041E4DAu, 0xF129AF75u, 12));
            H16(TW) = (unsigned short)(H16(TW) + 1);
            x += 5035;
        } while (x < 1022001);
        y += 4348;
    } while (y < 600001);
}
