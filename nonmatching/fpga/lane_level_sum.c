/*
 * lane_level_sum -- stock 0x4004ACC8 (260 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sum over n raw samples of one lane of (sample x q - base), negative terms counted as 0, with q = gain[V/div] x SYS_INFO+0x26C / 1000 and base = q x SYS_INFO+0x1B8. Optionally (start) re-arms an acquisition of n x 1.2 samples first. Stock quirks kept: for ch > 1 the lane number is used as the gain, and the 64-bit difference takes base >> 31 (logical) as its high word (unreachable: base <= 4294967 x 255 < 2^31).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"
#include "sys_info.h"

u32 lane_level_sum(u32 start, u32 lane, u32 ch, u32 n, unsigned char *buf)
{
    u32 gain, q, base, sum = 0, i;
    if (ch == 0) gain = SYS32(0x1C4 + 4 * SI(ch1_vdiv_index));
    else if (ch == 1) gain = SYS32(0x214 + 4 * SI(ch2_vdiv_index));
    else gain = lane;
    SI(runtime_quotient) = n / 10;
    q = (gain * SI(acq_geometry_26c)) / 1000;
    base = q * SI(f_1b8);
    if (start) {
        SYS32(0x30) = (n * 12) / 10;
        SYS32(0x34) = 0;
        fpga_sample_count_write((n * 12) / 10);
        acquisition_prepare();
        acq_wait_done();
    }
    capture_with_length(lane, n, (u32)buf);
    for (i = 0; i < n; i++) {
        unsigned long long p = (unsigned long long)buf[i] * q;
        u32 lo = (u32)p, hi = (u32)(p >> 32);
        u32 dlo = lo - base;
        u32 dhi = hi - (base >> 31) - (lo < base);
        if ((int)dhi < 0) dlo = 0;
        sum += dlo;
    }
    return sum;
}
