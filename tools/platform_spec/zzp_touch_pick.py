# Choice of the object dragged on the plot: distance from the touch point to each trace, the
# veneers per channel and the picker that plot_touch_drag (batch 71) calls on a press.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define U 0x42E68998u                /* drag state */
#define TCH 0x42E68BB0u              /* touch point: +4 / +6 now */
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define S16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))
"""

HIT = COM + """
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
"""

wr(0x400695D0, 648, "trace_hit_main",
   "Distance from the touch point (0x42E68BB0 + 4 / + 6, relative to the plot box SYS+4 / +6) to the trace of channel ch (0: buffer 0x413F92E8, result word 0x42E68998 + 0x38, "
   "range words 0x40545990; else buffer + 0x927C0, result + 0x3C, range 0x405459AC). The columns within the radius word 0x42E68998 + 4 (clamped to 0 .. width SYS+8) are "
   "scanned when the touch x lies inside SYS+0x10 + 2 .. SYS+0x12 - 2; each column has 1 + 2 x half SYS+0x550 x word SYS+0x5F8 samples, sample row height SYS+0xA - "
   "(sample >> SYS+0x1BC), and the smallest sqrt(dx^2 + dy^2) wins (dy = 0 when the trace crosses the touch row). When the radius is below the best trace distance found and the touch row lies "
   "inside the band between the two range words (rows height - range, divided by 3 in split view), the result is the distance to the nearer band edge instead. In split view a "
   "touch below height / 3 gives 0x7FFFFFFF.",
   HIT + """
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
""")

wr(0x40069858, 660, "trace_hit_zoom",
   "As trace_hit_main for the lower (zoomed) part of the split view: ch 2 uses buffer 0x413F92E8 + 0x124F80, result 0x42E68998 + 0x40 and range words 0x40545990; any other ch "
   "buffer + 0x1B7740, result + 0x44 and range 0x405459AC. Samples per column 1 + 2 x half SYS+0x550 x word SYS+0x610, touch x window SYS+0x16 + 2 .. SYS+0x18 - 2. The band "
   "rows are height / 3 + 2 x (height - range) / 3, and a touch above height / 3 gives 0x7FFFFFFF.",
   HIT + """
void trace_hit_zoom(u32 ch)
{
    u32 out = ch == 2 ? U + 0x40 : U + 0x44;
    u32 buf = ch == 2 ? TRACES + 0x124F80u : TRACES + 0x1B7740u;
    u32 rng = ch == 2 ? 0x40545990u : 0x405459ACu, h, third, a, b;
    int n = 1 + 2 * (int)(H16(SYS + 0x550) * W32(SYS + 0x610));
    int rx = S16(TCH + 4) - H16(SYS + 4), ty = S16(TCH + 6) - H16(SYS + 6), best;

    best = trace_dist(buf, n, rx, ty, H16(SYS + 0x16), H16(SYS + 0x18));
    h = H16(SYS + 0xA);
    third = (h * 0xAAABu) >> 17;
    if (W32(U + 4) < (u32)best) {
        b = third + (2 * (h - W32(rng))) / 3;
        a = third + (2 * (h - W32(rng + 4))) / 3;
        best = band(best, (int)b, (int)a, ty);
    }
    if (ty < (int)third)
        best = 0x7FFFFFFF;
    W32(out) = best;
}
""")

for a, nm, tgt, ch in ((0x40069AEC, "trace_hit_ch1", "trace_hit_main", 0), (0x40069AF4, "trace_hit_ch2", "trace_hit_main", 1),
                       (0x40069AFC, "trace_hit_zoom_ch1", "trace_hit_zoom", 2), (0x40069B04, "trace_hit_zoom_ch2", "trace_hit_zoom", 3)):
    wr(a, 8, nm, "%s(%d)." % (tgt, ch), """
void %s(u32 ch);

void %s(void)
{
    %s(%d);
}
""" % (tgt, nm, tgt, ch))

wr(0x40069B0C, 492, "touch_drag_pick",
   "Picks the object a plot press drags and stores it in byte 0x42E68998 (codes of plot_touch_drag): 0 when the touch point 0x42E68BB0 + 4 / + 6 is (0, 0); 13 when SYS+0x652 "
   "is set; else the nearest cursor within the radius word 0x42E68998 + 4: SYS+0x802 -> touch_hit_cursor_x, words + 0x48 / + 0x4C give 7 / 8; SYS+0x803 -> "
   "touch_hit_cursor_y, + 0x50 / + 0x54 give 9 / 10; SYS+0x3DA52C -> touch_hit_x_b, + 0x58 / + 0x5C give 11 / 12 (unsigned, a later equal distance does not win). Without a "
   "cursor, the nearest trace: CH2 (SYS+0x102) 4, its zoom part 6 in split view, CH1 (SYS+0x3E) 3, its zoom part 5. When both channels are on and the chosen trace and the "
   "other one of the same part are both under 10 px away, the result is 3 when word SYS+0xB30 < SYS+0xB4C, else 4.",
   COM + """
void touch_hit_cursor_x(void);
void touch_hit_cursor_y(void);
void touch_hit_x_b(void);
void trace_hit_ch1(void);
void trace_hit_ch2(void);
void trace_hit_zoom_ch1(void);
void trace_hit_zoom_ch2(void);

#define SPLIT B8(SYS + 0x2AA)

static u32 pick(void)
{
    u32 m = 0, best = W32(U + 4), o;

    if (B8(SYS + 0x802)) {
        touch_hit_cursor_x();
        for (o = 0; o < 2; o++)
            if (W32(U + 0x48 + 4 * o) < best) {
                best = W32(U + 0x48 + 4 * o);
                m = 7 + o;
            }
    }
    if (B8(SYS + 0x803)) {
        touch_hit_cursor_y();
        for (o = 0; o < 2; o++)
            if (W32(U + 0x50 + 4 * o) < best) {
                best = W32(U + 0x50 + 4 * o);
                m = 9 + o;
            }
    }
    if (B8(SYS + 0x3DA52C)) {
        touch_hit_x_b();
        for (o = 0; o < 2; o++)
            if (W32(U + 0x58 + 4 * o) < best) {
                best = W32(U + 0x58 + 4 * o);
                m = 11 + o;
            }
    }
    if (m)
        return m;
    best = 0x7FFFFFFF;
    if (B8(SYS + 0x102)) {
        trace_hit_ch2();
        if (W32(U + 0x3C) < best) {
            best = W32(U + 0x3C);
            m = 4;
        }
        if (SPLIT) {
            trace_hit_zoom_ch2();
            if (W32(U + 0x44) < best) {
                best = W32(U + 0x44);
                m = 6;
            }
        }
    }
    if (!B8(SYS + 0x3E))
        return m;
    trace_hit_ch1();
    if (W32(U + 0x38) < best) {
        best = W32(U + 0x38);
        m = 3;
    }
    if (SPLIT) {
        trace_hit_zoom_ch1();
        if (W32(U + 0x40) < best)
            m = 5;
    }
    if (!B8(SYS + 0x3E) || !B8(SYS + 0x102))
        return m;
    if (m == 3 || m == 4) {
        if (W32(U + 0x38) >= 10 || W32(U + 0x3C) >= 10)
            return m;
    } else if (m == 5 || m == 6) {
        if (W32(U + 0x40) >= 10 || W32(U + 0x44) >= 10)
            return m;
    } else
        return m;
    return W32(SYS + 0xB30) < W32(SYS + 0xB4C) ? 3 : 4;
}

void touch_drag_pick(void)
{
    if (S16(TCH + 4) == 0 && S16(TCH + 6) == 0) {
        B8(U) = 0;
        return;
    }
    B8(U) = B8(SYS + 0x652) ? 13 : pick();
}
""")
