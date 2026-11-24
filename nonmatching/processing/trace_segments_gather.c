/*
 * trace_segments_gather -- stock 0x4003C5CC (452 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Builds the display samples of trace tr for screen columns start .. start + n: with z = the samples per column (mode 0/1: +0x5F8, else +0x610), c = 2z + 1 values per segment and column, and per segment k the offset o[k] = SYS+0x38C[k] - SYS+0x2C4[k] - z x width / 2: dest[(col x segs + k) x c + j] = src[k x SYS+0x2B0 + z x col + j + o[k]] for j < c, the source index clamped to 0..0x493E0 unless z equals the record setting SYS+0x5D4 (then no offset, no clamp). src = 0x411AF3E8 + tr x 0x927C0, dest = 0x413F92E8 + tr x 0x927C0. At most 101 segments (stock stack array).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define P(o) (SYS + 0x5F4 + (o))
#define TRBUF(ch) (0x411AF3E8u + (ch) * 0x927C0u)

void trace_segments_gather(u32 tr, u32 start, u32 n)
{
    int off[101];
    u32 segs = H16(P(0)), rec = SI(block_points), rl = SI(f_5d4), end = start + n + 1, z, c;
    volatile unsigned short *src, *dst;
    int i, k, j, half, si;
    z = mode_not_0_or_1(tr) ? W32(P(0x1C)) : W32(P(4));
    c = 1 + 2 * z;
    if ((int)segs > 0) {
        half = (int)(z * SI(plot_width));
        half /= 2;
        for (k = 0; k < (int)segs; k++)
            off[k] = (int)(W32(SYS + 0x38C + 4 * k) - W32(SYS + 0x2C4 + 4 * k)) - half;
    }
    src = (volatile unsigned short *)TRBUF(tr);
    dst = (volatile unsigned short *)(0x413F92E8u + tr * 0x927C0u);
    for (i = (int)start; i < (int)end; i++) {
        if ((int)segs <= 0)
            continue;
        for (k = 0; k < (int)segs; k++) {
            if ((int)c <= 0)
                continue;
            for (j = 0; j < (int)c; j++) {
                si = (int)((u32)k * rec + z * (u32)i + (u32)j);
                if (z != rl) {
                    si += off[k];
                    if (si < 0)
                        si = 0;
                    else if (si > 0x493E0)
                        si = 0x493E0;
                }
                dst[(u32)i * segs * c + (u32)k * c + (u32)j] = src[si];
            }
        }
    }
}
