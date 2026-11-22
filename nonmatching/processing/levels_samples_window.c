/*
 * levels_samples_window -- stock 0x40061A70 (820 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, fits the visible part of the level samples to the plot width w = s32 0x42E68668+0xB8: with n = +0xF8, bin = s64 +0x100 and span = s64 +0x110, the first / last index i = (((s64 +0x120 / +0x128) - bin) x n / 2) / span; clears 0x42CC64B4 (n + 2 u16) and n u16 at 0x42CC64B4 + 2n + 2 (n / 2) - 10; c = i1 - i0 + 1 samples from *(0x42E68CB0) + 2 i0 + 2. If w < c: peak_hold_decimate(src, w, c / w) and copy w samples back to the start of the buffer. Otherwise the samples (minus 0x8000) are centred at index 500000 of the zoom workspace 0x416431E8 with 100 copies of each end sample beyond them, zoomed to w points by the nearest-neighbour (m = w x SYS+0x554 / c below 3 x SYS+0x554) or the spline resampler, offset back by 0x8000 into 0x4191F8A8 and copied to the buffer start.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))

#define BLK 0x42E68668u
#define SB 0x42CC64B4u
#define PTR 0x42E68CB0u
#define OUT 0x416431E8u
#define MID (OUT + 0xF4240u)
#define FIN 0x4191F8A8u

unsigned long long __aeabi_uldivmod(unsigned long long n, unsigned long long d);
void fill_u16(unsigned short *d, unsigned short v, u32 n);
void peak_hold_decimate(volatile short *buf, u32 n, double step);
void floating_point_interpolation_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift);

void levels_samples_window(void)
{
    long long bin, span, i0, i1, c, k;
    u32 n, nn, src, m, h2, wh;
    int w;
    volatile unsigned short *d, *p;
    if (B8(0x42E68BB0u + 0x17))
        return;
    n = W32(BLK + 0xF8);
    bin = S64(BLK + 0x100);
    span = S64(BLK + 0x110);
    i0 = __aeabi_ldivmod(__aeabi_ldivmod((S64(BLK + 0x120) - bin) * (long long)(int)n, 2), span);
    i1 = __aeabi_ldivmod(__aeabi_ldivmod((S64(BLK + 0x128) - bin) * (long long)(int)n, 2), span);
    fill_u16((unsigned short *)SB, 0, n + 2);
    nn = W32(BLK + 0xF8);
    fill_u16((unsigned short *)(SB + nn * 2 + (u32)(((int)nn / 2) * 2) - 10), 0, nn);
    c = i1 - i0 + 1;
    src = W32(PTR) + (u32)i0 * 2 + 2;
    w = (int)W32(BLK + 0xB8);
    if ((long long)w < c) {
        peak_hold_decimate((volatile short *)src, (u32)w, (double)c / (double)(long long)w);
        copy_u16((unsigned short *)W32(PTR), (const unsigned short *)src, (u32)w);
        return;
    }
    m = (u32)__aeabi_uldivmod((unsigned long long)(long long)w * (unsigned long long)W32(SYS + 0x554),
                              (unsigned long long)c);
    h2 = (u32)__aeabi_ldivmod(c, 2);
    if (c >= 1) {
        d = (volatile unsigned short *)(MID - 2 * h2);
        p = (volatile unsigned short *)src;
        k = c;
        do {
            *d++ = (unsigned short)(*p++ - 0x8000);
        } while (--k);
    }
    fill_u16((unsigned short *)(MID - 2 * h2 - 200), H16(MID - 2 * h2), 100);
    fill_u16((unsigned short *)(MID + 2 * h2), H16(MID + 2 * h2 - 2), 100);
    if (m < 3 * W32(SYS + 0x554))
        simple_integer_index_mapping_nearest_neighbour(OUT, 500000, (u32)w & 0xFFFF, m, 0, 0);
    else
        floating_point_interpolation_resampling(OUT, 500000, (u32)w & 0xFFFF, m, 0LL);
    wh = (u32)__aeabi_ldivmod((long long)w, 2);
    if ((long long)w >= 1) {
        d = (volatile unsigned short *)(FIN - 2 * wh);
        p = (volatile unsigned short *)(MID - 2 * wh);
        k = w;
        do {
            *d++ = (unsigned short)((short)*p++ - 32768);
        } while (--k);
    }
    copy_u16((unsigned short *)W32(PTR), (const unsigned short *)(FIN - 2 * wh), (u32)w);
}
