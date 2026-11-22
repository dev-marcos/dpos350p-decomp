/*
 * trace_hit_main -- stock 0x400695D0 (648 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Distance from the touch point (0x42E68BB0 + 4 / + 6, relative to the plot box SYS+4 / +6) to the trace of channel ch (0: buffer 0x413F92E8, result word 0x42E68998 + 0x38, range words 0x40545990; else buffer + 0x927C0, result + 0x3C, range 0x405459AC). The columns within the radius word 0x42E68998 + 4 (clamped to 0 .. width SYS+8) are scanned when the touch x lies inside SYS+0x10 + 2 .. SYS+0x12 - 2; each column has 1 + 2 x half SYS+0x550 x word SYS+0x5F8 samples, sample row height SYS+0xA - (sample >> SYS+0x1BC), and the smallest sqrt(dx^2 + dy^2) wins (dy = 0 when the trace crosses the touch row). When the radius is below the best trace distance found and the touch row lies inside the band between the two range words (rows height - range, divided by 3 in split view), the result is the distance to the nearer band edge instead. In split view a touch below height / 3 gives 0x7FFFFFFF.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

#define SYS 0x40544E68u
#define U 0x42E68998u                /* drag state */
#define TCH 0x42E68BB0u              /* touch point: +4 / +6 now */
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define S16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define TRACES 0x413F92E8u           /* trace sample buffers */
typedef double (*sqrt_fn)(double);
#define SQRT_T ((sqrt_fn)0x4003DF75u)   /* sqrt at 0x4003DF74 is Thumb */

static int clamp_x(int v)
{
    int w = H16(SYS + 8);

    if (v < 0)
        return 0;
    return w > v ? v : w;
}

static u32 sample_y(u32 s)
{
    u32 sh = W32(SYS + 0x1BC) & 0xFF;

    return H16(SYS + 0xA) - (sh >= 32 ? 0 : s >> sh);
}

/* smallest distance from (rx, ty) to the trace columns x0 .. x1 - 1 (n samples each) */
static int trace_dist(u32 buf, int n, int rx, int ty, u32 xlo, u32 xhi)
{
    int best = 0x0FFFFFFF, prev = 0x7FFFFFFF, x0, x1, x, j, y, dy, d, r = (int)W32(U + 4);
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

void trace_hit_main(u32 ch)
{
    u32 out = ch == 0 ? U + 0x38 : U + 0x3C;
    u32 buf = ch == 0 ? TRACES : TRACES + 0x927C0u;
    u32 rng = ch == 0 ? 0x40545990u : 0x405459ACu, h, a, b;
    int n = 1 + 2 * (int)(H16(SYS + 0x550) * W32(SYS + 0x5F8));
    int rx = S16(TCH + 4) - H16(SYS + 4), ty = S16(TCH + 6) - H16(SYS + 6), best;

    best = trace_dist(buf, n, rx, ty, H16(SYS + 0x10), H16(SYS + 0x12));
    if (W32(U + 4) < (u32)best) {
        h = H16(SYS + 0xA);
        a = h - W32(rng + 4);
        b = h - W32(rng);
        if (B8(SYS + 0x2AA)) {
            b /= 3;
            a /= 3;
        }
        best = band(best, (int)b, (int)a, ty);
    }
    if (B8(SYS + 0x2AA) && (int)((H16(SYS + 0xA) * 0xAAABu) >> 17) < ty)
        best = 0x7FFFFFFF;
    W32(out) = best;
}
