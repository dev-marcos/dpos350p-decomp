/*
 * channel_post_process -- stock 0x4004006C (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Post-processing of one channel (0 = CH1, needs SYS+0x3E; 1 = CH2, needs SYS+0x102) while SYS+0x554 < SYS+0x55C: time-base step x 1.3, measurement class, resampling of the channel buffer (SYS+0x7B4 + 4 ch) and the final normalisation.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"

void channel_post_process(u32 ch)
{
    u32 w;
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
    } else if (ch == 1) {
        if (SYS8(0x102) == 0)
            return;
    }
    if (SYS32(0x554) >= SYS32(0x55C))
        return;
    timebase_apply_2b0_x1_3();
    w = SYS32(0x7BC);
    amplitude_statistical_normalization(ch);
    workspace_copy_and_resampling(ch, SYS32(0x7B4 + 4 * ch), w >> 1, w & 0xFFFF, SYS32(0x55C));
    final_per_channel_normalization(ch);
}
