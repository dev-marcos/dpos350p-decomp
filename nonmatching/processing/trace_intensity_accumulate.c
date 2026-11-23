/*
 * trace_intensity_accumulate -- stock 0x4005EDC0 (540 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Adds the current capture of trace tr (u8) to its hit-count buffer 0x400954A4 + tr x 0x12BE70 (u16, H = u16 SYS+0xA entries per column), only when u16 SYS+0x14 is set. For each of the u16 SYS+0x5F4 segments (W32 SYS+0x2B0 points each, from offset 0) and each point j = 10 .. points - 11, with a / b the u16 values at 0x411AF3E8 / +0x927C0: column (SI(plot_width) - H) / 2 + (a >> s), row ((L x H - b) >> s) - 1 with s = SYS+0x1BC, L = SYS+0x1C0; the 3 x 3 cells around it get +5, the centre +15. Then the largest non-zero count over the columns u16 SYS+0x10 + 1 .. u16 SYS+0x12 - 2, rows 20 .. H - 41, divided by 200, becomes the intensity divisor SYS+0x7B0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define HITS(tr) (0x400954A4u + (tr) * 0x12BE70u)
#define PTS 0x411AF3E8u

#define INC(i, k) (H16(base + 2u * (u32)(i)) = (unsigned short)(H16(base + 2u * (u32)(i)) + (k)))

void trace_intensity_accumulate(u32 tr)
{
    u32 h = SI(plot_height), lh, y0, y1, nseg, lim, o, j, a, b, base, mx, col, v;
    int off, sh, idx, hm, r, k, cnt;
    volatile unsigned short *p;
    lh = SI(acq_geometry_1c0) * h;
    off = ((int)SI(plot_width) - (int)h) / 2;
    y1 = SI(f_012);
    y0 = SI(f_010);
    if (SI(f_014) == 0)
        return;
    tr &= 0xFF;
    nseg = SI(f_5f4);
    if (nseg) {
        base = HITS(tr);
        lim = SI(f_2b0) - 10;
        sh = (int)SI(f_1bc);
        o = 0;
        do {
            u32 step = SI(f_2b0);
            if (lim >= 11) {
                p = (volatile unsigned short *)(PTS + 2 * o + 0x14);
                j = 10;
                do {
                    a = p[0];
                    b = H16((u32)p + 0x927C0);
                    p++;
                    j++;
                    idx = (int)(h * (u32)(off + ((int)a >> sh))) + ((int)(lh - b) >> sh) - 1 - (int)h;
                    INC(idx, 5);
                    INC(idx + 1, 5);
                    INC(idx + 2, 5);
                    idx += (int)h;
                    INC(idx, 5);
                    INC(idx + 1, 15);
                    INC(idx + 2, 5);
                    idx += (int)h;
                    INC(idx, 5);
                    INC(idx + 1, 5);
                    INC(idx + 2, 5);
                } while (j < lim);
            }
            o += step;
        } while (--nseg);
    }
    hm = (int)h - 40;
    r = (int)y0 + 1;
    mx = 0;
    if (r < (int)y1 - 1) {
        col = h * (u32)r;
        cnt = (int)y1 - 1 - (int)y0 - 1;
        do {
            if (hm > 20) {
                p = (volatile unsigned short *)(HITS(tr) + col * 2 + 0x28);
                k = hm - 20;
                do {
                    v = *p++;
                    if (v != 0 && v >= mx)
                        mx = v;
                } while (--k);
            }
            col += h;
        } while (--cnt);
    }
    SI(f_7b0) = mx / 200;
}
