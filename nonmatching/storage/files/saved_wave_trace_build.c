/*
 * saved_wave_trace_build -- stock 0x40036864 (244 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Rebuilds the trace of a saved waveform. State at SYS+0x3DA530: byte +0 buffer index (buffers of 0x927C0 bytes from 0x411AF3E8), word +4 first column, word +8 end column, word +0xC set to end - first. For each column x in [first, end): the mean of n = SI(samples_per_column) u16 samples starting at SI(block_base) + n * (x - u16 SYS+8 / 2), divided by SI(acq_geometry_1c0); u16 SYS+0xA minus that goes to u16 SYS+0x3DA548 + 2 * (x - first).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define ST (SYS + 0x3DA530u)

void saved_wave_trace_build(void)
{
    u32 base = 0x411AF3E8u + 0x927C0u * B8(ST);
    int x = (int)W32(ST + 4);
    W32(ST + 0xC) = W32(ST + 8) - W32(ST + 4);
    for (; x < (int)W32(ST + 8); x++) {
        u32 n = SI(samples_per_column), i;
        volatile unsigned short *p = (volatile unsigned short *)(base + 2 * (SI(block_base) + n * ((u32)x - (SI(plot_width) >> 1))));
        unsigned long long sum = 0;
        long long v;
        for (i = 0; i < n; i++)
            sum += p[i];
        v = __aeabi_ldivmod((long long)sum, (long long)n);
        v = __aeabi_ldivmod(v, (long long)SI(acq_geometry_1c0));
        H16(SYS + 0x3DA548u + 2 * ((u32)x - W32(ST + 4))) = (unsigned short)(SI(plot_height) - (u32)v);
    }
}
