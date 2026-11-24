/*
 * trace_intensity_draw_window -- stock 0x4005DF5C (332 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Intensity view over the capture window, only when SYS+0x6FC >= SYS+0x708: columns SYS+0x700 .. SYS+0x700 + u16 SYS+0x6F8 of trace tr drawn as palette[16 + (v / SYS+0x7B0) x 1000 / 1333] (palette as in trace_intensity_draw, without the SYS+0x7B0 update), then fb_present_rect over the window. (The symbol table splits this function at 0x4005E0AC; the entry here covers both pieces.)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "stock_asm.h"
#include "ui/draw.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u

#define HITS(tr) (0x400954A4u + (tr) * 0x12BE70u)
#define FBP (*(volatile u32 *)0x40095498u)
#define PAL_A 0x42E45864u
#define PAL_B 0x42E47B8Cu

void trace_intensity_draw_window(u32 tr)
{
    u32 w700, x0, y0, rows, pal, out, c0, c1, n, off, src, dst, row, v, q, ax, bx, cy;
    int po;
    if (SI(runtime_fine_progress) < SI(runtime_progress_threshold))
        return;
    w700 = SI(runtime_coarse_progress);
    x0 = SI(plot_x0);
    c1 = SI(runtime_coarse_step) + w700;
    ax = w700 + x0 + 1;
    bx = SI(runtime_coarse_step) + ax;
    y0 = SI(plot_y0);
    rows = SI(plot_height);
    cy = rows + y0;
    if (SI(f_652)) {
        pal = (SI(palette_alt) ? PAL_B : PAL_A) + 6000;
    } else {
        po = (int)(short)3000 * (int)(short)mode_not_0_or_2(tr);
        pal = (SI(palette_alt) ? PAL_B : PAL_A) + (u32)po;
    }
    out = FBP + (x0 + 1) * 2 + y0 * 1022 * 2;
    c0 = w700;
    if (c0 < c1) {
        out += c0 * 2;
        off = rows * c0;
        for (n = c1 - c0; n; n--, out += 2, off += rows) {
            if (rows == 0)
                continue;
            src = HITS(tr & 0xFF) + off * 2;
            dst = out;
            for (row = rows; row; row--, dst += 0x7FC, src += 2) {
                v = H16(src);
                if (v == 0)
                    continue;
                q = v / SI(intensity_divisor) * 1000;
                H16(dst) = H16(pal + ((u32)(((unsigned long long)q * 0xC4A83BECu) >> 32) >> 10) * 2 + 0x20);
            }
        }
    }
    fb_present_rect(ax & 0xFFFF, y0, bx & 0xFFFF, cy & 0xFFFF);
}
