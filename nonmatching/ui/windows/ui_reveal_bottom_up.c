/*
 * ui_reveal_bottom_up -- stock 0x4003D46C (296 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * As ui_reveal_slide_down, but the top n rows of the new image are copied to the bottom n rows of the rect, so the window rises from the bottom edge.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/windows.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
#define FB_W     1022u
#define fb_front ((unsigned short *)0x41FB8FE8u)
#define fb_back  ((unsigned short *)0x41BFBF68u)
#define fb_off   ((unsigned short *)0x4233CBBCu)
typedef double (*tanh_fn)(double);
#define TANH_T ((tanh_fn)0x4005F91Du)   /* tanh at 0x4005F91C is Thumb */

void ui_reveal_bottom_up(volatile unsigned short *r)
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
            const unsigned short *src = fb_off + y * FB_W + x;
            unsigned short *dst = fb_back + (y + h - n) * FB_W + x;
        if (n) {
            for (i = n; i; i--) {
                for (j = 0; j < w; j++)
                    dst[j] = src[j];
                src += FB_W;
                dst += FB_W;
            }
        }
        }
        fb_present_rect(x, (y + h - n) & 0xFFFF, (x + w) & 0xFFFF, (y + h) & 0xFFFF);
    } while (n < h);
    ui_window_redraw_dirty();
}
