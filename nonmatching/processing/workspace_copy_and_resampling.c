/*
 * workspace_copy_and_resampling -- stock 0x4005C7BC (348 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Zoom of channel ch's u16 trace src while SYS+0x554 < n: the len samples centred on half (from half - len/2, plus a tail of 15 / 7 / 4 / 2 / 1 for CH2 at time-base index SYS+0x588 = 12..16) are copied to the workspace 0x41A13AE8, resampled by the method of byte SYS+0x51C (CH1) / +0x51D (CH2): 0 nearest neighbour, 1 cubic spline, other windowed sinc (shift 0), and copied back (CH2 shifted by the tail).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define F32(a) (*(volatile float *)(a))

#define WS 0x41A13AE8u

void workspace_copy_and_resampling(u32 ch, u32 src, u32 half, u32 len, u32 n)
{
    int lo, hi, e, i;
    u32 mode, tail = 0;
    if (W32(SYS + 0x554) >= n)
        return;
    lo = (int)(half - (len >> 1));
    mode = ch == 0 ? B8(SYS + 0x51C) : B8(SYS + 0x51D);
    hi = lo + (int)len;
    if (ch == 1)
        switch (B8(SYS + 0x588)) {
        case 12: tail = 15; break;
        case 13: tail = 7; break;
        case 14: tail = 4; break;
        case 15: tail = 2; break;
        case 16: tail = 1; break;
        }
    e = (int)tail + hi;
    for (i = lo; i < e; i++)
        H16(WS + 2 * (u32)i) = H16(src + 2 * (u32)i);
    if (mode == 0)
        simple_integer_index_mapping_nearest_neighbour(WS, half, len, n, 0, 0);
    else if (mode == 1)
        floating_point_interpolation_resampling(WS, half, len, n, 0);
    else
        fixed_point_weighted_or_filtered_resampling(WS, half, len, n, 0);
    for (i = lo; i < hi; i++)
        H16(src + 2 * (u32)i) = H16(WS + 2 * (tail + (u32)i));
}
