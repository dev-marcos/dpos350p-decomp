/*
 * xy_grid_draw -- stock 0x4002C5F8 (416 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Grid of the XY display: a square of side H = u16 SYS+0xA centred horizontally in the trace area (x = SYS+4 + (SYS+8 - H) / 2, y = SYS+6). When the byte SYS+0x655 is set, grid lines every u16 SYS+0x20 pixels in 0x303030 and 40 ticks 12.6 pixels apart on the two centre lines; then the border in 0x505050.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void vline_draw(u32 x, u32 y0, u32 y1);
#define H16(a) (*(volatile unsigned short *)(a))
#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

void xy_grid_draw(void)
{
    u32 side = H16(SYS + 0xA), top = H16(SYS + 6), step;
    int left = (int)H16(SYS + 4) + ((int)(H16(SYS + 8) - side) / 2);
    int right = (int)side + left, bottom = (int)(side + top), cy = (int)(top + (side >> 1));
    int cx = left + (int)(side >> 1) + 1, p, i;
    float base;
    step = H16(SYS + 0x20);
    if (*(volatile unsigned char *)(SYS + 0x655)) {
        draw_colour_set(0x303030);
        for (p = left; p <= right; p += step)
            vline_draw(U16(p), top, U16(bottom));
        for (p = (int)top; p <= bottom; p += step)
            func_4002ecc4_span(U16(p), U16(left), U16(right));
        base = (float)(int)top;
        for (i = 0; i < 40; i++)
            func_4002ecc4_span(U16((int)(base + (float)i * 12.6f)), U16(cx - 2), U16(cx + 2));
        base = (float)left;
        for (i = 0; i < 40; i++)
            vline_draw(U16((int)(base + (float)i * 12.6f)), U16(cy - 1), U16(cy + 3));
    }
    draw_colour_set(0x505050);
    vline_draw(U16(left), top, U16(bottom));
    vline_draw(U16(right), top, U16(bottom));
    func_4002ecc4_span(top, U16(left), U16(right));
    func_4002ecc4_span(U16(bottom), U16(left), U16(right));
}
