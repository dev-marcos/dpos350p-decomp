/*
 * window_64_recalc -- stock 0x40061680 (292 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Block 0x42E68668, by mode word +0x18C: 1 / 2 take the 64-bit limits a = +0x128, b = +0x120 and set the centre +0x130 = (a + b) / 2 and the span +0x138 = a - b; 3 limits the span to twice the centre, 4 moves the centre to span / 2 when twice the centre is below the span; both then set +0x120 / +0x128 = centre -/+ span / 2. Centre and span are always rewritten.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/windows.h"

#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))

void window_64_recalc(void)
{
    long long span = M64(0x138), c = M64(0x130), t, h;
    int mode = *(volatile int *)(0x42E68668u + 0x18C);
    if (mode == 1 || mode == 2) {
        long long a = M64(0x128), b = M64(0x120);
        c = (long long)((unsigned long long)b + (unsigned long long)a) / 2;
        span = (long long)((unsigned long long)a - (unsigned long long)b);
    } else if (mode == 3 || mode == 4) {
        t = (long long)((unsigned long long)c * 2u);
        if (t < span) {
            if (mode == 3)
                span = t;
            else
                c = span / 2;
        }
        h = span / 2;
        M64(0x120) = (long long)((unsigned long long)c - (unsigned long long)h);
        M64(0x128) = (long long)((unsigned long long)c + (unsigned long long)h);
    }
    M64(0x130) = c;
    M64(0x138) = span;
}
