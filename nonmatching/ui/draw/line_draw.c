/*
 * line_draw -- stock 0x4002ED90 (216 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Line from (x0, y0) to (x1, y1) as vertical segments: slope k = (y1 - y0) * 10000 / (x1 - x0) (0 when x1 = x0, as the sdiv instruction gives), and for each x from x0 towards x1 (inclusive) a vline_draw between the previous and the new y = y0 + (x - x0) * k / 10000.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void vline_draw(u32 x, u32 y0, u32 y1);

void line_draw(int x0, int y0, int x1, int y1)
{
    int d = x1 - x0, k = d ? (int)((u32)(y1 - y0) * 10000u) / d : 0;
    int prev = y0, x = x0, y;
    if (x1 >= x0) {
        do {
            y = y0 + (int)((u32)(x - x0) * (u32)k) / 10000;
            if (y >= prev)
                vline_draw((unsigned short)x, (unsigned short)prev, (unsigned short)y);
            else
                vline_draw((unsigned short)x, (unsigned short)y, (unsigned short)prev);
            x++;
            prev = y;
        } while (x1 >= x);
    } else {
        do {
            y = y0 + (int)((u32)(x - x0) * (u32)k) / 10000;
            if (y >= prev)
                vline_draw((unsigned short)x, (unsigned short)prev, (unsigned short)y);
            else
                vline_draw((unsigned short)x, (unsigned short)y, (unsigned short)prev);
            x--;
            prev = y;
        } while (x >= x1);
    }
}
