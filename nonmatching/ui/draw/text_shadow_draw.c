/*
 * text_shadow_draw -- stock 0x4002D0D4 (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the string s at (x, y) over an n x m block of copies offset by 1..n / 1..m pixels drawn in colour alpha << 24; the drawing colour (word 0x42E68664) is restored before the final copy at (x, y).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

#define W32(a) (*(volatile u32 *)(a))
#define H16(a) (*(volatile unsigned short *)(a))

void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha)
{
    u32 saved = W32(0x42E68664u), xx, j;
    draw_colour_set((u32)alpha << 24);
    for (xx = (unsigned short)x + 1; n; n--, xx++)
        for (j = 0; j < m; j++)
            text_draw((unsigned short)xx, (unsigned short)(y + 1 + j), s);
    draw_colour_set(saved);
    text_draw((unsigned short)x, y, s);
}
