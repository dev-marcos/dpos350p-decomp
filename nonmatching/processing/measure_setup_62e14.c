/*
 * measure_setup_62e14 -- stock 0x40062E14 (200 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Block 0x42E68668: byte +0 = 0, runs 0x400612FC and 0x40060FE8, byte +5 = 1, runs 0x40060DC8 and 0x40061F10; then d = (+0x10 - (+0x20 + +0x28) / 2) / 6 into +0x140 and +0x10 + d / 2 into +0x148; v = (s64)+0x48: +0x138 = 2v, +0x130 = v, +0x120 = v - 2v / 2, +0x128 = v + 2v / 2.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "processing.h"
#include "stock_asm.h"

#define MD(o) (*(volatile double *)(0x42E68668u + (o)))
#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))

void measure_setup_62e14(void)
{
    double d;
    long long v, t;
    *(volatile unsigned char *)0x42E68668u = 0;
    levels_view_layout();
    auto_level_search();
    *(volatile unsigned char *)(0x42E68668u + 5) = 1;
    fft_spur_cancel();
    func_40061f10();
    d = (MD(0x10) - (MD(0x20) + MD(0x28)) * 0.5) / 6.0;
    MD(0x140) = d;
    MD(0x148) = MD(0x10) + d * 0.5;
    v = (long long)MD(0x48);
    t = (long long)((unsigned long long)v * 2u);
    M64(0x138) = t;
    M64(0x130) = v;
    M64(0x120) = (long long)((unsigned long long)v - (unsigned long long)(t / 2));
    M64(0x128) = (long long)((unsigned long long)v + (unsigned long long)(M64(0x138) / 2));
}
