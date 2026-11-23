/*
 * trace_intensity_draw -- stock 0x4005DDD4 (392 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Intensity (persistence) view of trace tr: for each of the u16 SYS+8 columns and SYS+0xA rows of the plot (origin u16 SYS+4 + 1, SYS+6), a non-zero hit count v (u16, 0x400954A4 + tr x 0x12BE70, column-major) is drawn as palette[2 + sqrt(byte SYS+0x654) x byte SYS+0x654 x 22 / 1000 + ((v / SYS+0x7B0) x 100 + 1) / 110]. Palette: 0x42E45864 (byte SYS+0x653 = 0) or 0x42E47B8C, offset 6000 bytes in the single-trace mode (byte SYS+0x652), else 3000 x mode_not_0_or_2(tr); in that second case with SYS+0x653 set, SYS+0x7B0 is divided by 3 (stored). The D-cache is cleaned first.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "stock_asm.h"
#include "ui/draw.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))

#define HITS(tr) (0x400954A4u + (tr) * 0x12BE70u)
#define FBP (*(volatile u32 *)0x40095498u)
#define PAL_A 0x42E45864u
#define PAL_B 0x42E47B8Cu

typedef double (*sqrt_fn)(double);
#define SQRT_T ((sqrt_fn)0x4003DF75u)   /* sqrt at 0x4003DF74 is Thumb */

void trace_intensity_draw(u32 tr)
{
    u32 x1 = SI(plot_x0) + 1, y0 = SI(plot_y0), cols = SI(plot_width), rows = SI(plot_height), pal, k, base, out,
        col, row, off, src, dst, v, q, idx;
    int po;
    if (SI(f_652)) {
        pal = (SI(f_653) ? PAL_B : PAL_A) + 6000;
    } else {
        po = (int)(short)3000 * (int)(short)mode_not_0_or_2(tr);
        if (SI(f_653)) {
            pal = PAL_B + (u32)po;
            SI(f_7b0) = SI(f_7b0) / 3;
        } else
            pal = PAL_A + (u32)po;
    }
    k = SI(f_654);
    base = 2 + (u32)SQRT_T((double)k) * k * 22 / 1000;
    dcache_clean_inv_all();
    out = FBP + x1 * 2 + y0 * 1022 * 2;
    off = 0;
    for (col = cols; col; col--, out += 2, off += rows) {
        if (rows == 0)
            continue;
        src = HITS(tr & 0xFF) + off * 2;
        dst = out;
        for (row = rows; row; row--, dst += 0x7FC, src += 2) {
            v = H16(src);
            if (v == 0)
                continue;
            q = v / SI(f_7b0) * 100;
            idx = base + ((u32)(((unsigned long long)q * 0x94F2094Fu + 0x94F2094Fu) >> 32) >> 6);
            H16(dst) = H16(pal + idx * 2);
        }
    }
}
