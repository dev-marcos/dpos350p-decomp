/*
 * font_metrics_build -- stock 0x4002DCAC (196 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads a font's glyph data header (big-endian u16 count a and u16 height b at +0x4BC): for each non-zero byte p[i] of the a bytes after the header, stores i + 1 in the u16 table d+0xA.. and, from the second on, the distance to the previous one in d+0x260..; then d+2 = a, d+4 = b, d+6 = number of glyphs - 1, d+8 = largest distance - 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void font_metrics_build(volatile unsigned char *d)
{
    const unsigned char *p = *(const unsigned char * volatile *)(d + 0x4BC);
    u32 a = p[1] | (p[0] << 8), b = p[3] | (p[2] << 8);
    int end = (unsigned short)(a + 4), i;
    unsigned short n = 0, mx = 0;
    for (i = 4; i < end; i++) {
        if (p[i]) {
            volatile unsigned short *h = (volatile unsigned short *)(d + n * 2);
            h[5] = i + 1;
            if (n) {
                unsigned short v = 0xFFFF + (i + 1) - h[4];
                h[0x130] = v;
                if (mx < v)
                    mx = v;
            }
            n++;
        }
    }
    *(volatile unsigned short *)(d + 2) = a;
    *(volatile unsigned short *)(d + 4) = b;
    *(volatile unsigned short *)(d + 6) = 0xFFFF + n;
    *(volatile unsigned short *)(d + 8) = mx;
    *(const unsigned char * volatile *)(d + 0x4BC) = p;
}
