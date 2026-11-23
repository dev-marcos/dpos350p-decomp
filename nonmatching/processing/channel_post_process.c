/*
 * channel_post_process -- stock 0x4004006C (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Post-processing of one channel (0 = CH1, needs SYS+0x3E; 1 = CH2, needs SYS+0x102) while SYS+0x554 < SYS+0x55C: time-base step x 1.3, measurement class, resampling of the channel buffer (SYS+0x7B4 + 4 ch) and the final normalisation.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"
#include "sys_info.h"

void channel_post_process(u32 ch)
{
    u32 w;
    if (ch == 0) {
        if (SI(ch1_enabled) == 0)
            return;
    } else if (ch == 1) {
        if (SI(ch2_enabled) == 0)
            return;
    }
    if (SI(f_554) >= SI(f_55c))
        return;
    timebase_apply_2b0_x1_3();
    w = SI(f_7bc);
    amplitude_statistical_normalization(ch);
    workspace_copy_and_resampling(ch, SYS32(0x7B4 + 4 * ch), w >> 1, w & 0xFFFF, SI(f_55c));
    final_per_channel_normalization(ch);
}
