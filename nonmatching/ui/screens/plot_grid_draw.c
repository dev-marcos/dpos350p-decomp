/*
 * plot_grid_draw -- stock 0x4002C7AC (1188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the plot grid of the main view (x0 = SYS+4, y0 = SYS+6, width SYS+8, height SYS+0xA). With the grid byte SYS+0x655 set it draws, in colour 0x303030, vertical lines every u16 SYS+0x1C and horizontal lines every u16 SYS+0x20 from the top-left corner, ticks of 5 pixels across the vertical centre line every 12.6 pixels (40 of them; every 4.2 and 8.4 in the upper and lower split parts) and ticks from centre - 1 to centre + 3 on the horizontal centre line every 19 pixels (50). Then the frame in colour 0x505050. With the split-screen byte SYS+0x2AA set the plot is cut in an upper part of height h/3 + 1 (line step SYS+0x20 / 3) and a lower part from y0 + h/3 + 1 of height 2h/3 + 1 (line step 2 x SYS+0x20 / 3), each with its own grid; the upper part gets the frame without its bottom line.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define LAY 0x42E64520u              /* UI layout table */
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define ENT(n) (TXT + 0x1A0 * (n))

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);

void vline_draw(u32 x, u32 y0, u32 y1);
void func_4002ecc4_span(u32 y, u32 x0, u32 x1);

/* grid lines and centre ticks of one part (ya..yb, horizontal centre line at cy) */
static void grid_part(int x0, int x1, int cx, int ya, int yb, int cy, u32 xs, u32 ys, float st)
{
    int p, k;
    float f;

    draw_colour_set(0x303030);
    for (p = x0; p <= x1; p += xs)
        vline_draw(p & 0xFFFF, ya & 0xFFFF, yb & 0xFFFF);
    for (p = ya; p <= yb; p += ys)
        func_4002ecc4_span(p & 0xFFFF, x0, x1 & 0xFFFF);
    f = (float)ya;
    for (k = 0; k < 40; k++)
        func_4002ecc4_span((int)(f + (float)k * st) & 0xFFFF, (cx - 2) & 0xFFFF, (cx + 2) & 0xFFFF);
    f = (float)x0;
    for (k = 0; k < 50; k++)
        vline_draw((int)(f + (float)k * 19.0f) & 0xFFFF, (cy + 0xFFFF) & 0xFFFF, (cy + 3) & 0xFFFF);
}

static void frame(int x0, int x1, int ya, int yb)
{
    draw_colour_set(0x505050);
    vline_draw(x0, ya & 0xFFFF, yb & 0xFFFF);
    vline_draw(x1 & 0xFFFF, ya & 0xFFFF, yb & 0xFFFF);
    func_4002ecc4_span(ya & 0xFFFF, x0, x1 & 0xFFFF);
    func_4002ecc4_span(yb & 0xFFFF, x0, x1 & 0xFFFF);
}

void plot_grid_draw(void)
{
    int x0 = H16(SYS + 4), w = H16(SYS + 8);
    int cx = x0 + (w >> 1), x1 = x0 + w;
    u32 xs = H16(SYS + 0x1C);
    int split = B8(SYS + 0x2AA), grid = B8(SYS + 0x655);
    u32 ys = H16(SYS + 0x20);
    int y0 = H16(SYS + 6), h = H16(SYS + 0xA);
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
    func_4002ecc4_span(y0, x0, x1 & 0xFFFF);
    h = H16(SYS + 0xA);
    ya = H16(SYS + 6) + h / 3 + 1;
    t = (2 * h) / 3;
    yb = ya + t + 1;
    if (B8(SYS + 0x655))
        grid_part(x0, x1, cx, ya, yb, ya + t / 2, xs, (2 * (u32)H16(SYS + 0x20)) / 3, 0x1.0ccccep+3f);    /* 8.4, bits 0x41066667 */
    frame(x0, x1, ya, yb);
}
