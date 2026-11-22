/*
 * bode_curves_zoom -- stock 0x40039918 (744 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, zooms the gain and the phase curve of the Bode view (u16 rows 0x42E180C8+0x8D20 / +0x9CC0, W32 +0x5C points) to u16 +4 pixels: each curve is centred at index 10000 of the workspace 0x416431E8 with 100 copies of its end samples, resampled (m = +4 x SYS+0x554 / +0x5C, kept in +0x40; nearest neighbour below 3 x SYS+0x554, else spline) and copied back; then every row is clamped to 3 .. u16 +6 - 3 and flipped to +6 - row.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "bode.h"
#include "processing.h"
#include "util.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define U64(a) (*(volatile unsigned long long *)(a))

#define BB 0x42E180C8u
#define WS 0x416431E8u
#define MID (WS + 0x4E20u)

static void zoom_one(u32 off)
{
    int c = (int)W32(BB + 0x5C);
    u32 n;
    copy_u16((unsigned short *)(MID - 2 * (u32)(c / 2)), (const unsigned short *)(BB + off), (u32)c + 100);
    c = (int)W32(BB + 0x5C);
    fill_u16((unsigned short *)(MID - 200 - 2 * (u32)(c / 2)), H16(BB + off), 100);
    c = (int)W32(BB + 0x5C);
    fill_u16((unsigned short *)(MID + 2 * (u32)(c / 2)), H16(BB + 2 * (u32)c + off - 2), 100);
    if (W32(BB + 0x40) < 3 * W32(SYS + 0x554))
        simple_integer_index_mapping_nearest_neighbour(WS, 10000, H16(BB + 4), W32(BB + 0x40), 0, 0);
    else
        floating_point_interpolation_resampling(WS, 10000, H16(BB + 4), W32(BB + 0x40), 0LL);
    n = H16(BB + 4);
    copy_u16((unsigned short *)(BB + off), (const unsigned short *)(MID - (n & ~1u)), n);
}

static void clamp_flip(u32 off)
{
    volatile unsigned short *p = (volatile unsigned short *)(BB + off);
    u32 i, h;
    if (H16(BB + 4) == 0)
        return;
    i = 0;
    do {
        if ((short)*p < 3) {
            *p = 3;
        } else {
            h = H16(BB + 6);
            if ((int)(short)h - 3 < (int)(short)*p)
                *p = (unsigned short)(0xFFFD + h);
        }
        *p = (unsigned short)(H16(BB + 6) - *p);
        p++;
        i++;
    } while (i < H16(BB + 4));
}

void bode_curves_zoom(void)
{
    if (B8(0x42E68BB0u + 0x17))
        return;
    W32(BB + 0x40) = H16(BB + 4) * W32(SYS + 0x554) / W32(BB + 0x5C);
    zoom_one(0x8D20);
    zoom_one(0x9CC0);
    clamp_flip(0x8D20);
    clamp_flip(0x9CC0);
}
