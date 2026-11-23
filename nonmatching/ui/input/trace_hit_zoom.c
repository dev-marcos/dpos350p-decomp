/*
 * trace_hit_zoom -- stock 0x40069858 (660 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * As trace_hit_main for the lower (zoomed) part of the split view: ch 2 uses buffer 0x413F92E8 + 0x124F80, result 0x42E68998 + 0x40 and range words 0x40545990; any other ch buffer + 0x1B7740, result + 0x44 and range 0x405459AC. Samples per column 1 + 2 x half SYS+0x550 x word SYS+0x610, touch x window SYS+0x16 + 2 .. SYS+0x18 - 2. The band rows are height / 3 + 2 x (height - range) / 3, and a touch above height / 3 gives 0x7FFFFFFF.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "sys_info.h"
#include "state/touch_ctl.h"

#define SYS 0x40544E68u
#define U 0x42E68998u                /* drag state */
#define TCH 0x42E68BB0u              /* touch point: +4 / +6 now */
#define S16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define TRACES 0x413F92E8u           /* trace sample buffers */
typedef double (*sqrt_fn)(double);
#define SQRT_T ((sqrt_fn)0x4003DF75u)   /* sqrt at 0x4003DF74 is Thumb */

static int clamp_x(int v)
{
    int w = SI(plot_width);

    if (v < 0)
        return 0;
    return w > v ? v : w;
}

static u32 sample_y(u32 s)
{
    u32 sh = SI(f_1bc) & 0xFF;

    return SI(plot_height) - (sh >= 32 ? 0 : s >> sh);
}

/* smallest distance from (rx, ty) to the trace columns x0 .. x1 - 1 (n samples each) */
static int trace_dist(u32 buf, int n, int rx, int ty, u32 xlo, u32 xhi)
{
    int best = 0x0FFFFFFF, prev = 0x7FFFFFFF, x0, x1, x, j, y, dy, d, r = (int)TCS(cursor_limit);
    volatile unsigned short *p;

    x0 = clamp_x(rx - r);
    x1 = clamp_x(r + rx);
    if ((int)xlo + 2 >= rx || rx >= (int)xhi - 2 || x0 >= x1)
        return best;
    for (x = x0; x < x1; x++) {
        p = (volatile unsigned short *)(buf + 2 * (n * x));
        for (j = 0; j < n; j++, p++) {
            y = (int)sample_y(*p);
            if (prev != 0x7FFFFFFF) {
                if (prev < ty)
                    dy = ty < y ? 0 : y - ty;
                else if (y >= ty || ty >= prev)
                    dy = y - ty;
                else
                    dy = 0;
                d = (int)SQRT_T((double)(int)((u32)dy * (u32)dy + (u32)(x - rx) * (u32)(x - rx)));
                if (d <= best)
                    best = d;
            }
            prev = y;
        }
    }
    return best;
}

static int band(int best, int b, int a, int ty)
{
    int mid = (b + a) / 2;

    if (b >= ty || ty >= a)
        return best;
    return mid < ty ? a - ty : ty - b;
}

void trace_hit_zoom(u32 ch)
{
    u32 out = ch == 2 ? U + 0x40 : U + 0x44;
    u32 buf = ch == 2 ? TRACES + 0x124F80u : TRACES + 0x1B7740u;
    u32 rng = ch == 2 ? 0x40545990u : 0x405459ACu, h, third, a, b;
    int n = 1 + 2 * (int)(SI(capture_count) * SI(f_610));
    int rx = S16(TCH + 4) - SI(plot_x0), ty = S16(TCH + 6) - SI(plot_y0), best;

    best = trace_dist(buf, n, rx, ty, SI(f_016), SI(f_018));
    h = SI(plot_height);
    third = (h * 0xAAABu) >> 17;
    if (TCS(cursor_limit) < (u32)best) {
        b = third + (2 * (h - W32(rng))) / 3;
        a = third + (2 * (h - W32(rng + 4))) / 3;
        best = band(best, (int)b, (int)a, ty);
    }
    if (ty < (int)third)
        best = 0x7FFFFFFF;
    W32(out) = best;
}
