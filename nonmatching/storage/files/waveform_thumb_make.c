/*
 * waveform_thumb_make -- stock 0x40035EE0 (528 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Builds the preview of a saved waveform from the trace area (x u16 SYS+4, y u16 SYS+6, width u16 SYS+8, height u16 SYS+0xA) of the scanned-out framebuffer 0x41FB8FE8, in steps of 4.685 x 3.645 pixels (5 x 4 samples per output pixel), both ends included. Same layout as screenshot_thumb_make (u16 width and height at SYS+0x109684, pixels from SYS+0x109688); each channel is the sum x 1e6 / 4685 / 3645, clamped. Nothing but the header word is written when the area has a negative height.
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

void waveform_thumb_make(void)
{
    volatile unsigned short *fb = (volatile unsigned short *)0x41FB8FE8u;
    volatile unsigned short *dst = (volatile unsigned short *)(SYS + 0x109688u);
    int x0 = (int)(1000u * H16(SYS + 4)), x1 = x0 + (int)(1000u * H16(SYS + 8));
    int y0 = (int)(1000u * H16(SYS + 6)), y1 = y0 + (int)(1000u * H16(SYS + 0xA));
    int x, y, i, j, k = 0;
    W32(TW) = 0;
    if (y1 < y0)
        return;
    y = y0;
    do {
        H16(TW) = 0;
        H16(TW + 2) = (unsigned short)(H16(TW + 2) + 1);
        if (x1 >= x0) {
            x = x0;
            do {
                u32 r = 0, g = 0, b = 0;
                for (i = 0; i < 4; i++) {
                    u32 row = 1022u * (u32)((y + 1000 * i) / 1000);
                    for (j = 0; j < 5; j++) {
                        u32 p = fb[(u32)((x + 1000 * j) / 1000) + row];
                        r += p >> 11 & 0x1F;
                        g += p >> 5 & 0x3F;
                        b += p & 0x1F;
                    }
                }
                dst[k++] = pack(scale(r, 0xDFD0C9F6u, 0x8FD66C05u, 11), scale(g, 0xDFD0C9F6u, 0x8FD66C05u, 11),
                                scale(b, 0xDFD0C9F6u, 0x8FD66C05u, 11));
                H16(TW) = (unsigned short)(H16(TW) + 1);
                x += 4685;
            } while (x1 >= x);
        }
        y += 3645;
    } while (y1 >= y);
}
