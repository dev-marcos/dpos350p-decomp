/*
 * plot_grid_draw -- stock 0x4002C7AC (1188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the plot grid of the main view (x0 = SYS+4, y0 = SYS+6, width SYS+8, height SYS+0xA). With the grid byte SYS+0x655 set it draws, in colour 0x303030, vertical lines every u16 SYS+0x1C and horizontal lines every u16 SYS+0x20 from the top-left corner, ticks of 5 pixels across the vertical centre line every 12.6 pixels (40 of them; every 4.2 and 8.4 in the upper and lower split parts) and ticks from centre - 1 to centre + 3 on the horizontal centre line every 19 pixels (50). Then the frame in colour 0x505050. With the split-screen byte SYS+0x2AA set the plot is cut in an upper part of height h/3 + 1 (line step SYS+0x20 / 3) and a lower part from y0 + h/3 + 1 of height 2h/3 + 1 (line step 2 x SYS+0x20 / 3), each with its own grid; the upper part gets the frame without its bottom line.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define ENT(n) (TXT + 0x1A0 * (n))

/* grid lines and centre ticks of one part (ya..yb, horizontal centre line at cy) */
static void grid_part(int x0, int x1, int cx, int ya, int yb, int cy, u32 xs, u32 ys, float st)
{
    int p, k;
    float f;

    draw_colour_set(0x303030);
    for (p = x0; p <= x1; p += xs)
        vline_draw(p & 0xFFFF, ya & 0xFFFF, yb & 0xFFFF);
    for (p = ya; p <= yb; p += ys)
        hspan_fill(p & 0xFFFF, x0, x1 & 0xFFFF);
    f = (float)ya;
    for (k = 0; k < 40; k++)
        hspan_fill((int)(f + (float)k * st) & 0xFFFF, (cx - 2) & 0xFFFF, (cx + 2) & 0xFFFF);
    f = (float)x0;
    for (k = 0; k < 50; k++)
        vline_draw((int)(f + (float)k * 19.0f) & 0xFFFF, (cy + 0xFFFF) & 0xFFFF, (cy + 3) & 0xFFFF);
}

static void frame(int x0, int x1, int ya, int yb)
{
    draw_colour_set(0x505050);
    vline_draw(x0, ya & 0xFFFF, yb & 0xFFFF);
    vline_draw(x1 & 0xFFFF, ya & 0xFFFF, yb & 0xFFFF);
    hspan_fill(ya & 0xFFFF, x0, x1 & 0xFFFF);
    hspan_fill(yb & 0xFFFF, x0, x1 & 0xFFFF);
}

void plot_grid_draw(void)
{
    int x0 = SI(plot_x0), w = SI(plot_width);
    int cx = x0 + (w >> 1), x1 = x0 + w;
    u32 xs = SI(f_01c);
    int split = SI(zoom_on), grid = SI(grid_on);
    u32 ys = SI(grid_step);
    int y0 = SI(plot_y0), h = SI(plot_height);
    int ya, yb, t;

    if (!split) {
        if (grid)
            grid_part(x0, x1, cx, y0, y0 + h, y0 + (h >> 1), xs, ys, 0x1.933334p+3f);    /* 12.6 */
        frame(x0, x1, y0, y0 + h);
        return;
    }
    yb = y0 + h / 3 + 1;
    if (grid)
        grid_part(x0, x1, cx, y0, yb, y0 + h / 6, xs, ys / 3, 0x1.0ccccep+2f);    /* 4.2, bits 0x40866667 */
    draw_colour_set(0x505050);
    vline_draw(x0, y0, yb & 0xFFFF);
    vline_draw(x1 & 0xFFFF, y0, yb & 0xFFFF);
    hspan_fill(y0, x0, x1 & 0xFFFF);
    h = SI(plot_height);
    ya = SI(plot_y0) + h / 3 + 1;
    t = (2 * h) / 3;
    yb = ya + t + 1;
    if (SI(grid_on))
        grid_part(x0, x1, cx, ya, yb, ya + t / 2, xs, (2 * (u32)SI(grid_step)) / 3, 0x1.0ccccep+3f);    /* 8.4, bits 0x41066667 */
    frame(x0, x1, ya, yb);
}
