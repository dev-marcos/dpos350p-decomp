/*
 * split_window_marker_draw -- stock 0x40042520 (616 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set or SYS+0x2AA is 0: takes the zoom window of width H16 SYS+0x24 centred at H16 SYS+0x26 inside the plot (x SYS+4, y SYS+6, width SYS+8, height SYS+0xA), clamps its edges to [1, w-2] and [0, w-1], dims the plot left and right of it (colour 0xCCFFFFFF, a third of the plot height) and draws each edge as three vertical lines 0xAAAAAA / 0x555555 / 0x222222 shading inwards. (The left edge is clamped against w-2, not x+w-2.)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u

#define H3 ((H16(SYS + 0xA) * 0xAAABu) >> 17)    /* a third of the plot height */
#define VL(col, x) do { draw_colour_set(col); vline_draw((u32)(x) & 0xFFFF, (H16(SYS + 6) + 1) & 0xFFFF, (H16(SYS + 6) + H3) & 0xFFFF); } while (0)

void split_window_marker_draw(void)
{
    int half, c, lo, hi, w, x4, lo2, hi2;

    if (g_touch->serviced)
        return;
    if (SI(zoom_on) == 0)
        return;
    half = SI(f_024) >> 1;
    c = SI(f_026);
    lo = c - half;
    hi = c + half;
    w = SI(plot_width);
    if (lo <= 0)
        lo = 1;
    if (w - 2 < lo)
        lo = w - 2;
    if (w - 1 < hi)
        hi = w - 1;
    if (hi < 0)
        hi = 0;
    draw_colour_set(0xCCFFFFFFu);
    rect_fill((SI(plot_x0) + 1) & 0xFFFF, (SI(plot_y0) + 1) & 0xFFFF, (u32)lo & 0xFFFF, H3);
    rect_fill((u32)(hi + SI(plot_x0)) & 0xFFFF, (SI(plot_y0) + 1) & 0xFFFF, (u32)(SI(plot_width) - hi) & 0xFFFF, H3);

    x4 = SI(plot_x0);
    w = SI(plot_width);
    lo2 = x4 + lo;
    if (lo2 < x4 + 2)
        lo2 = x4 + 2;
    if (w - 2 < lo2)
        lo2 = w - 2;
    hi2 = x4 + hi;
    if (hi2 < x4 + 2)
        hi2 = x4 + 2;
    if (w + x4 - 2 < hi2)
        hi2 = w + x4 - 2;
    VL(0xAAAAAAu, lo2);
    VL(0x555555u, lo2 + 1);
    VL(0x222222u, lo2 + 2);
    VL(0xAAAAAAu, hi2);
    VL(0x555555u, hi2 - 1);
    VL(0x222222u, hi2 - 2);
}
