/*
 * manual_cursors_draw -- stock 0x4003117C (1424 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The manual cursors, unless the touch byte 0x42E68BB0+0x17 or SYS+0x652 is set, or SYS+0x28 is set while SYS+0x3C is 0. Byte SYS+0x802 enables the two time cursors H1 / H2 (words SYS+0x808 / SYS+0x80C, x from the trace left edge), byte SYS+0x803 the two level cursors V1 / V2 (words SYS+0x810 / SYS+0x814, y from the top). Each cursor is a dotted grey 0x888888 line (dash 3, period 4); the H cursors get a 17 x 14 marker with a triangle and 'H1' / 'H2' at the top, the V cursors a triangle and 'V1' / 'V2' at the right edge (x = 0x39E .. 0x3B6), kept 8 pixels inside the trace area. With the zoom byte SYS+0x2AA set, the area starts 2 pixels right and a third of its height lower, and the V positions are scaled by 2/3.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

static void hmark(u32 xc, u32 top, const char *digit, unsigned short ty1, unsigned short ty2, u32 dx2)
{
    u32 y = top + 0xB, j;
    draw_colour_set(0xDDDDDD);
    rect_fill(U16(xc - 8), U16(top), 0x11, 0xE);
    font_set(0x42E65418u);
    for (j = 0; j <= 8; j++) {
        vline_draw(U16(xc - j), U16(y), U16(y + 0xD - j));
        vline_draw(U16(xc + j), U16(y), U16(y + 0xD - j));
    }
    draw_colour_set(0);
    text_draw(U16(xc - 3), ty1, "H");
    text_draw(U16(xc - dx2), ty2, digit);
}

static int clamp_y(int y)
{
    int lo = (int)SI(plot_y0) + 8, hi = (int)(SI(plot_y0) + SI(plot_height)) - 8;
    if (y < lo)
        return lo;
    return hi > y ? y : hi;
}

static void vmark(int pos)
{
    int j;
    draw_colour_set(0xDDDDDD);
    for (j = 0; j <= 8; j++) {
        hspan_fill(U16(pos - j), U16(0x39E + j), 0x3AA);
        hspan_fill(U16(pos + j), U16(0x39E + j), 0x3AA);
    }
    rect_fill(0x3AB, U16(pos - 8), 0xB, 0x11);
}

void manual_cursors_draw(void)
{
    u32 x0, y0, right, bottom, h, h1, h2, v1, v2, i;
    int nx, ny, p;
    if (g_touch->serviced)
        return;
    if (SI(f_652))
        return;
    if (SI(acq_path_mode) && SI(acq_transition_flag) == 0)
        return;
    if (SI(cursor_x_on) == 0 && SI(cursor_y_on) == 0)
        return;
    x0 = SI(plot_x0);
    y0 = SI(plot_y0);
    h = SI(plot_height);
    right = x0 + SI(plot_width);
    bottom = y0 + h;
    h1 = SI(cursor_x1);
    h2 = SI(cursor_x2);
    v1 = SI(cursor_y1);
    v2 = SI(cursor_y2);
    if (SI(zoom_on) == 0) {
        x0 += 1;
        y0 += 2;
    } else {
        x0 += 2;
        y0 = y0 + ((h * 0xAAABu) >> 17) + 3;
        v1 = (u32)((int)(v1 << 1) / 3);
        v2 = (u32)((int)(v2 << 1) / 3);
    }
    nx = (int)(right - x0 + 1) / 4;
    ny = (int)(bottom - y0 + 1) / 4;
    draw_colour_set(0x888888);
    if (SI(cursor_x_on) && ny > 0) {
        u32 xa = U16(h1 + x0 + 0xFFFF), xb = h2 + x0 + 0xFFFF;
        for (i = 0; (int)i < ny; i++)
            vline_draw(xa, U16(y0 + 4 * i), U16(y0 + 4 * i + 2));
        for (i = 0; (int)i < ny; i++)
            vline_draw(U16(xb), U16(y0 + 4 * i), U16(y0 + 4 * i + 2));
    }
    if (SI(cursor_y_on) && nx > 0) {
        u32 ya = U16(v1 + y0 + 0xFFFF), yb = v2 + y0 + 0xFFFF;
        for (i = 0; (int)i < nx; i++)
            hspan_fill(ya, U16(x0 + 4 * i), U16(x0 + 4 * i + 2));
        for (i = 0; (int)i < nx; i++)
            hspan_fill(U16(yb), U16(x0 + 4 * i), U16(x0 + 4 * i + 2));
    }
    if (SI(cursor_x_on)) {
        u32 r7 = y0 - 1 + 0xB;
        hmark(h1 + SI(plot_x0), y0 - 1, "1", U16(r7 - 0xB), U16(r7 + 0xFFFE), 1);
        hmark(h2 + SI(plot_x0), y0 - 1, "2", U16(r7 - 0xB), U16(r7 + 0xFFFE), 2);
    }
    if (SI(cursor_y_on)) {
        p = clamp_y((int)(v1 + y0 - 1));
        vmark(p);
        text_spacing_set(2);
        font_set(0x42E65418u);
        draw_colour_set(0);
        text_draw(0x3A6, U16(p + 0xFFFB), "V1");
        p = clamp_y((int)(v2 + y0 - 1));
        vmark(p);
        text_spacing_set(1);
        font_set(0x42E65418u);
        draw_colour_set(0);
        text_draw(0x3A5, U16(p + 0xFFFB), "V2");
    }
}
