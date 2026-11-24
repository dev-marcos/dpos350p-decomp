/*
 * ui_reveal_slide_down -- stock 0x4003D594 (280 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Animated reveal of a window rect (record r: x +6, y +8, w +0xA, h +0xC) from the offscreen buffer into the back buffer: at each step the bottom n rows of the new image are copied to the top n rows of the rect and presented, so the window slides down into place. n = min(h, h x floor(10000 x (tanh(t) + 0.02)) / 10000), t from 0 in steps that start at 0.2 and shrink by 0.012 down to 0.01. Ends with ui_window_redraw_dirty.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/windows.h"
#include "shared/framebuffer.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
typedef double (*tanh_fn)(double);
#define TANH_T ((tanh_fn)0x4005F91Du)   /* tanh at 0x4005F91C is Thumb */

void ui_reveal_slide_down(volatile unsigned short *r)
{
    u32 x = r[3], y = r[4], w = r[5], h = r[6];
    double t = 0.0, s = 0.2;
    u32 n;
    do {
        double e = TANH_T(t) + 0.02;
        u32 q, i, j;
        t += s;
        s += -0.012;
        if (s < 0.01)
            s = 0.01;
        q = (u32)(e * 10000.0);
        q = q * h / 10000u;
        n = h > q ? q : h;
        {
            const unsigned short *src = fb_off + (y + h - n) * FB_W + x;
            unsigned short *dst = fb_back + y * FB_W + x;
        if (n) {
            for (i = n; i; i--) {
                for (j = 0; j < w; j++)
                    dst[j] = src[j];
                src += FB_W;
                dst += FB_W;
            }
        }
        }
        fb_present_rect(x, y, (x + w) & 0xFFFF, (n + y) & 0xFFFF);
    } while (n < h);
    ui_window_redraw_dirty();
}
