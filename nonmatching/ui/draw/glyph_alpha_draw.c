/*
 * glyph_alpha_draw -- stock 0x4002EE78 (624 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws an alpha-mask glyph g (u16 big-endian width and height, then width * height coverage bytes) at (x, y) on the screen *0x40095498 (stride 1022 pixels). Each coverage a is scaled by 255 - byte 0x42E6865C (when that is not 255), a >= 0xF6 stores the solid colour u16 0x42E68660, a < 3 is skipped, otherwise a is lowered by u16 0x42E68BE6 (when greater) and the colour R, G, B (bytes 0x42E6865D..F) is blended in: over black as a * C / 255, over a pixel as (a * C + (255 - a) * P) / 255 with P widened from RGB565 (v * 8 + 7, or 0; white = 255).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "shared/access.h"

void glyph_alpha_draw(u32 x, u32 y, const unsigned char *g)
{
    volatile unsigned char *c = (volatile unsigned char *)0x42E6865Cu;
    u32 w = ((u32)g[0] << 8) | g[1], h = ((u32)g[2] << 8) | g[3];
    int k = 255 - c[0], R = c[1], G = c[2], B = c[3];
    unsigned short solid = H16(0x42E68660u);
    u32 idx = x + y * 1022u;
    const unsigned char *src = g;
    if (h == 0)
        return;
    do {
        if (w) {
            volatile unsigned short *fb = (volatile unsigned short *)*(volatile u32 *)IMG(40095498);
            u32 j, p = idx;
            for (j = 0; j < w; j++, p++) {
                unsigned short d = fb[p];
                int a = src[4 + j], r, gg, b;
                if (k != 255)
                    a = (unsigned char)((a * k) / 255);
                if (a >= 0xF6) {
                    fb[p] = solid;
                    continue;
                }
                if (a < 3)
                    continue;
                if ((int)H16(0x42E68BE6u) < a)
                    a = (unsigned char)(a - H16(0x42E68BE6u));
                if (d == 0) {
                    r = a * R / 255;
                    gg = a * G / 255;
                    b = a * B / 255;
                } else {
                    int pr, pg, pb;
                    if (d == 0xFFFF) {
                        pr = pg = pb = 0xFF;
                    } else {
                        pr = (d >> 11) & 31;
                        pg = (d >> 5) & 63;
                        pb = d & 31;
                        if (pr)
                            pr = pr * 8 + 7;
                        if (pg)
                            pg = pg * 4 + 3;
                        if (pb)
                            pb = pb * 8 + 7;
                    }
                    r = (a * R + (255 - a) * (unsigned char)pr) / 255;
                    gg = (a * G + (255 - a) * (unsigned char)pg) / 255;
                    b = (a * B + (255 - a) * (unsigned char)pb) / 255;
                }
                fb[p] = (unsigned short)((((unsigned char)gg >> 2) << 5) | (((unsigned char)r >> 3) << 11) |
                                         ((unsigned char)b >> 3));
            }
        }
        idx += 1022;
        src += w;
    } while (--h);
}
