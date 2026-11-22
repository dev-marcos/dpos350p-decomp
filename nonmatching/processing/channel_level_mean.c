/*
 * channel_level_mean -- stock 0x4004ADF0 (284 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sum of lane_level_sum over lanes 0..7, / n / 8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"
#include "processing.h"

u32 channel_level_mean(u32 start, u32 ch, u32 n, unsigned char *buf)
{
    u32 s0, s1, s2, s3, s4, s5, s6, s7;
    s0 = lane_level_sum(start, 0, ch, n, buf);
    s1 = lane_level_sum(start, 1, ch, n, buf) + s0;
    s2 = lane_level_sum(start, 2, ch, n, buf);
    s3 = lane_level_sum(start, 3, ch, n, buf);
    s4 = lane_level_sum(start, 4, ch, n, buf);
    s5 = lane_level_sum(start, 5, ch, n, buf);
    s6 = lane_level_sum(start, 6, ch, n, buf);
    s6 = s6 + (s2 + s1 + s3 + s4 + s5);
    s7 = lane_level_sum(start, 7, ch, n, buf);
    return ((s7 + s6) / n) >> 3;
}
