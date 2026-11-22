/*
 * levels_view_layout -- stock 0x400612FC (632 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, lays out the level view in block 0x42E68668 from its bytes +1 / +2 (the two side panels): the plot box +0xB0 / +0xB4 / +0xB8 (width, also halved into +0xBC) / +0xC0 .. +0xD0 and the panel boxes +0xD4 .. +0xE0 / +0xE4 .. +0xF0. The FFT size +0xF8 becomes 0x1000 / 0x2000 / 0x4000 / 0x8000 for mode +0xF4 = 0..3 (kept otherwise), the sample pointer 0x42E68CB0 = 0x42CC64B4 + 2 x size, SYS+0x3C = 0, SYS+0x54E = 1. From the s64 sample rate SYS+0x538: bin width +0x100 = rate / 1000 / size, top +0x108 = (rate / 1000, unsigned) / 2, centre +0x118 = (top + bin) / 2, span +0x110 = top - bin.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))

#define BLK 0x42E68668u

void levels_view_layout(void)
{
    u32 n, f1, f2, x, y, w, c0, c4, c8, cc, d0;
    long long rate, bin, top;
    if (B8(0x42E68BB0u + 0x17))
        return;
    n = W32(BLK + 0xF8);
    switch (W32(BLK + 0xF4)) {
    case 0: n = 0x1000; break;
    case 1: n = 0x2000; break;
    case 2: n = 0x4000; break;
    case 3: n = 0x8000; break;
    }
    f1 = B8(BLK + 1);
    x = W32(BLK + 0xB0);
    y = W32(BLK + 0xB4);
    w = W32(BLK + 0xB8);
    c0 = W32(BLK + 0xC0);
    c4 = W32(BLK + 0xC4);
    c8 = W32(BLK + 0xC8);
    cc = W32(BLK + 0xCC);
    d0 = W32(BLK + 0xD0);
    f2 = B8(BLK + 2);
    if (!f1 && !f2) {
        x = 0x34; y = 0x30; w = 0x3C6; c0 = 0x1E0; c4 = 0xA1; c8 = 6; cc = 0x50; d0 = 6;
    } else if (f1) {
        x = 0x34; y = 0x30; w = 0x3C6;
        if (!f2) {
            cc = 0x2F;
            W32(BLK + 0xD4) = 0x34;
            c0 = 0x11A; c4 = 0xA1; c8 = 6; d0 = 6;
            W32(BLK + 0xD8) = 0x163;
            W32(BLK + 0xDC) = 0x3C6;
            W32(BLK + 0xE0) = 0xAB;
        } else {
            W32(BLK + 0xD4) = 0x34;
            c0 = 0x8F;
            W32(BLK + 0xE4) = 0x34;
            W32(BLK + 0xD8) = 0xD8;
            c4 = 0xA1;
            W32(BLK + 0xDC) = 0x3C6;
            c8 = 6;
            W32(BLK + 0xE0) = 0x8F;
            cc = 0x18; d0 = 6;
            W32(BLK + 0xE8) = 0x180;
            W32(BLK + 0xEC) = 0x3C6;
            W32(BLK + 0xF0) = 0x8F;
        }
    } else {
        cc = 0x2F; w = 0x3C6; x = 0x34; y = 0x30; c0 = 0x11A;
        W32(BLK + 0xE4) = 0x34;
        c4 = 0xA1; c8 = 6; d0 = 6;
        W32(BLK + 0xE8) = 0x163;
        W32(BLK + 0xEC) = 0x3C6;
        W32(BLK + 0xF0) = 0xAB;
    }
    W32(BLK + 0xCC) = cc;
    W32(BLK + 0xD0) = d0;
    W32(BLK + 0xB0) = x;
    W32(BLK + 0xB4) = y;
    W32(BLK + 0xC0) = c0;
    W32(BLK + 0xC4) = c4;
    W32(BLK + 0xC8) = c8;
    B8(SYS + 0x3C) = 0;
    B8(SYS + 0x54E) = 1;
    W32(0x42E68CB0u) = 0x42CC64B4u + n * 2;
    W32(BLK + 0xF8) = n;
    W32(BLK + 0xB8) = w;
    W32(BLK + 0xBC) = (u32)((int)w / 2);
    rate = S64(SYS + 0x538);
    bin = __aeabi_ldivmod(__aeabi_ldivmod(rate, 1000), (long long)(int)W32(BLK + 0xF8));
    top = (long long)(__aeabi_uldivmod((unsigned long long)rate, 1000) >> 1);
    S64(BLK + 0x108) = top;
    S64(BLK + 0x100) = bin;
    S64(BLK + 0x118) = __aeabi_ldivmod(top + bin, 2);
    S64(BLK + 0x110) = S64(BLK + 0x108) - S64(BLK + 0x100);
}
