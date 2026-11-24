/*
 * levels_update_chain -- stock 0x400623F4 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: when byte +4 of block 0x42E68668 is set runs 0x40061DB0 and 0x40061A70; then 0x4006053C, level_stats_5f010, level_stats_60008 and 0x4006233C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

void levels_update_chain(void)
{
    if (g_touch->serviced)
        return;
    if (*(volatile unsigned char *)(0x42E68668u + 4)) {
        fft_levels_db();
        levels_samples_window();
    }
    marker_values_compute();
    level_stats_5f010();
    level_stats_60008();
    level_stats_61000();
}
