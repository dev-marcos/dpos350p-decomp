/*
 * channel_moving_average -- stock 0x40040F18 (172 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Moving average of width w = SYS16+0x662 (w >= 2) over the n = SYS+0x7BC samples of the channel buffer (SYS+0x7B4 + 4 ch), written in place at the window start: buf[i - w + 1] = (buf[i - w + 1] + ... + buf[i]) / w (signed).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void channel_moving_average(u32 ch)
{
    volatile unsigned short *buf = (volatile unsigned short *)SYS32(0x7B4 + 4 * ch);
    int n = (int)SI(capture_samples), w, i, acc = 0;

    if (ch == 0) {
        if (SI(ch1_enabled) == 0)
            return;
    } else if (ch == 1) {
        if (SI(ch2_enabled) == 0)
            return;
    }

    w = SI(moving_avg_width);
    if (w < 2)
        return;
    for (i = 0; i < n; i++) {
        acc += buf[i];
        if (i >= w - 1) {
            int old = buf[i - w + 1];
            buf[i - w + 1] = acc / w;
            acc -= old;
        }
    }
}
