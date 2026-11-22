/*
 * lane_level_mean -- stock 0x4004ADCC (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * lane_level_sum / n.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"

u32 lane_level_mean(u32 start, u32 lane, u32 ch, u32 n, unsigned char *buf)
{
    return lane_level_sum(start, lane, ch, n, buf) / n;
}
