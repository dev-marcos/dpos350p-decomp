/*
 * icon_19c7c_draw -- stock 0x40019C7C (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws a small icon at (x, y) in colour word 0x409223EC: a 10 x 15 box at (x, y + 3) and a triangle of 8 spans pointing left, rows y + 10 +- k from x + 10 to x + 20 - k.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void func_4002ecc4(u32 a, u32 b, u32 c, u32 d);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void func_4002ce7c(u32 x, u32 y, const char *s);
void pixel_put(u32 x, u32 y);
#define W32(a) (*(volatile u32 *)(a))
#define H16(a) (*(volatile unsigned short *)(a))

void icon_19c7c_draw(u32 x, u32 y)
{
    int k;
    draw_colour_set(W32(0x409223ECu));
    func_4002ecc4(x, (unsigned short)(y + 3), 10, 15);
    for (k = 0; k < 8; k++) {
        func_4002ecc4_span((unsigned short)(y + 10 + k), (unsigned short)(x + 10), (unsigned short)(x + 20 - k));
        func_4002ecc4_span((unsigned short)(y + 10 - k), (unsigned short)(x + 10), (unsigned short)(x + 20 - k));
    }
}
