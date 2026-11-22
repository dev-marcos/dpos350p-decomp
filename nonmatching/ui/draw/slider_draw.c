/*
 * slider_draw -- stock 0x40019CF8 (184 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws a 410-pixel slider at (x, y) for v (0..255): filled part w = v * 410 / 100 (colour SYS+0x3DD57C), the rest (SYS+0x3DD578), and a 20 x 40 knob (SYS+0x3DD580) at x + clamp(w, 10, 400) - 10, y - 12.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void rect_fill(u32 a, u32 b, u32 c, u32 d);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void text_draw(u32 x, u32 y, const char *s);
void pixel_put(u32 x, u32 y);
#define W32(a) (*(volatile u32 *)(a))
#define H16(a) (*(volatile unsigned short *)(a))

void slider_draw(u32 x, u32 y, unsigned char v)
{
    volatile u32 *t = (volatile u32 *)(0x40544E68u + 0x3DD578u);
    int w = v * 410 / 100, c = w;
    if (c < 10)
        c = 10;
    else if (c > 400)
        c = 400;
    draw_colour_set(t[1]);
    rect_fill(x, y, w, 16);
    draw_colour_set(t[0]);
    rect_fill((unsigned short)(x + w), y, (unsigned short)(410 - w), 16);
    draw_colour_set(t[2]);
    rect_fill((unsigned short)(x + c - 10), (unsigned short)(y - 12), 20, 40);
}
