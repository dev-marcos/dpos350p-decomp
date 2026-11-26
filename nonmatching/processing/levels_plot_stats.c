/*
 * levels_plot_stats -- stock 0x400621EC (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For n = word +0xB8 samples of the s16 source (pointer 0x42E68CB0): v clamped to 1 .. top - 1 (top = word +0xC0), stores top - v as u16 at 0x42E5F010 and keeps the mean (+0x160), max (+0x164) and min (+0x168) of block 0x42E68668.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "state/levels_block.h"

#define MB32(o) (*(volatile int *)(0x42E68668u + (o)))
#define SRC (*(volatile short *volatile *)0x42E68CB0u)

static int sdv(int a, int b)
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int)(0u - (u32)a);
    return a / b;
}

void levels_plot_stats(void)
{
    int n = MB32(0xB8), top = MB32(0xC0), mx = -32767, mn = 32767, i, v;
    u32 sum = 0;
    volatile short *s = SRC;
    volatile unsigned short *d = (volatile unsigned short *)0x42E5F010u;
    for (i = 0; i < n; i++) {
        v = *s++;
        if (top - 1 < v)
            v = top - 1;
        else if (v <= 0)
            v = 1;
        v = top - v;
        *d++ = v;
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
        sum += v;
    }
    LEVELS(f_160) = sdv((int)sum, n);
    LEVELS(f_164) = mx;
    LEVELS(f_168) = mn;
}
