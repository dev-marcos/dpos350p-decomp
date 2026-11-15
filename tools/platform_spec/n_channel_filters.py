# Per-channel sample filters after unpacking: moving average, pairwise step limiter, copy into
# the display buffers and the frame-deviation blend selected by the time base. exec'd by gen_platform.py


def c(addr, size, name, desc, body, src="acquisition"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


GATE = """
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
    } else if (ch == 1) {
        if (SYS8(0x102) == 0)
            return;
    }
"""

c(0x40040F18, 172, "channel_moving_average",
  "Moving average of width w = SYS16+0x662 (w >= 2) over the n = SYS+0x7BC samples of the channel buffer (SYS+0x7B4 + 4 ch), written in place at the window start: buf[i - w + 1] = (buf[i - w + 1] + ... + buf[i]) / w (signed).", """
void channel_moving_average(u32 ch)
{
    volatile unsigned short *buf = (volatile unsigned short *)SYS32(0x7B4 + 4 * ch);
    int n = (int)SYS32(0x7BC), w, i, acc = 0;
""" + GATE + """
    w = SYS16(0x662);
    if (w < 2)
        return;
    for (i = 0; i < n; i++) {
        acc += buf[i];
        if (i >= w - 1) {
            int old = buf[i - w + 1];
            buf[i - w + 1] = acc / w;
            acc -= old;
        }
    }
}
""")
c(0x40040FC4, 204, "channel_step_limiter",
  "When SYS_INFO+0x70D is set: smooths steps between neighbouring samples. Each pair moves towards each other by half their difference, clamped to +-lim, lim = SYS+0x1C0 x (15 at V/div index 12, the most sensitive range, else 5); 6 passes at index 12, else 3. The V/div index is SYS+0x41 for CH1, SYS+0x105 otherwise. Touches samples 0..n.", """
void channel_step_limiter(u32 ch)
{
    volatile unsigned short *buf = (volatile unsigned short *)SYS32(0x7B4 + 4 * ch);
    int n = (int)SYS32(0x7BC), lim, passes, j;
    u32 vd;
    if (SYS8(0x70D) == 0)
        return;
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
        vd = SYS8(0x41);
    } else {
        if (ch == 1 && SYS8(0x102) == 0)
            return;
        vd = SYS8(0x105);
    }
    lim = (int)(SYS32(0x1C0) * (vd == 12 ? 15u : 5u));
    passes = vd == 12 ? 6 : 3;
    do {
        for (j = 0; j < n; j++) {
            int b = buf[j + 1], a = buf[j], d = a - b;
            if (lim <= d) d = lim;
            if (d < -lim) d = -lim;
            buf[j] = a - d / 2;
            buf[j + 1] = b + d / 2;
        }
    } while (--passes);
}
""")
c(0x40041094, 132, "channel_to_display_copy",
  "Copies SYS+0x454[k] samples of the channel buffer into the display buffer 0x42AC7CF4 + ch x 0x62250 at index SYS+0x2C4[k].", """
void channel_to_display_copy(u32 ch, u32 k)
{""" + GATE + """
    copy_u16((unsigned short *)(0x42AC7CF4u + ch * 0x62250u + SYS32(0x2C4 + 4 * k) * 2),
             (const unsigned short *)SYS32(0x7B4 + 4 * ch), SYS32(0x454 + 4 * k));
}
""")
c(0x40041118, 524, "frame_deviation_blend",
  "Blend of the F = SYS16+0x550 frames in the display buffer of a channel (frame k at index SYS+0x2C4[k], L = SYS+0x2B0 points): per point the mean over frames (to 0x42C947D4), per frame the largest deviation from the mean (starting at (1 - f) x lim, capped at lim = SYS+0x1C0 x n), then every sample = (w x sample + (100 - w) x mean) / 100 with w = 100 x deviation / lim. The stock code leaves the divisions by zero to the hardware (result 0); the C does the same explicitly.", """
void fill_u32(volatile u32 *d, u32 v, u32 n);

void frame_deviation_blend(u32 ch, u32 n, float fr)
{
    volatile unsigned short *buf = (volatile unsigned short *)(0x42AC7CF4u + ch * 0x62250u);
    volatile u32 *mean = (volatile u32 *)0x42C947D4u;
    volatile u32 *off = (volatile u32 *)(0x40544E68u + 0x2C4);
    int dev[100];
    int lim = (int)(SYS32(0x1C0) * n), init;
    u32 L = SYS32(0x2B0), F, j, k;
    init = (int)((1.0 - (double)fr) * (double)(float)lim);
    if (L != 0) {
        F = SYS16(0x550);
        for (j = 0; j < L; j++) {
            u32 s = 0;
            for (k = 0; k < SYS16(0x550); k++)
                s += buf[j + off[k]];
            mean[j] = F ? s / F : 0;
        }
    }
    fill_u32((volatile u32 *)dev, (u32)init, 100);
    F = SYS16(0x550);
    L = SYS32(0x2B0);
    for (j = 0; j < L; j++) {
        u32 m;
        if (F == 0)
            continue;
        m = mean[j];
        for (k = 0; k < SYS16(0x550); k++) {
            int d = (int)(buf[j + off[k]] - m);
            if (d < 0) d = -d;
            if (d < dev[k]) d = dev[k];
            dev[k] = d;
        }
    }
    for (k = 0; k < SYS16(0x550); k++)
        if (lim <= dev[k])
            dev[k] = lim;
    for (j = 0; j < SYS32(0x2B0); j++) {
        u32 m = mean[j];
        for (k = 0; k < SYS16(0x550); k++) {
            int w = lim ? (int)((u32)dev[k] * 100u) / lim : 0;
            u32 p = j + off[k];
            buf[p] = ((u32)(100 - w) * m + (u32)w * buf[p]) / 100;
        }
    }
}
""")
c(0x40041324, 144, "channel_filters_by_timebase",
  "Frame blend for the enabled channels at the fast time bases: index SYS8+0x588 < 16 uses n = 40 and f = 0.8, 16..20 uses n = 8 and f = 0.7; slower bases are left alone.", """
void frame_deviation_blend(u32 ch, u32 n, float fr);

void channel_filters_by_timebase(void)
{
    u32 tb = SYS8(0x588);
    if (tb < 16) {
        if (SYS8(0x3E))
            frame_deviation_blend(0, 0x28, 0.8f);
        if (SYS8(0x102))
            frame_deviation_blend(1, 0x28, 0.8f);
    } else if (tb < 21) {
        if (SYS8(0x3E))
            frame_deviation_blend(0, 8, 0.7f);
        if (SYS8(0x102))
            frame_deviation_blend(1, 8, 0.7f);
    }
}
""")
