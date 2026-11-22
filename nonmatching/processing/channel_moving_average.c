/*
 * channel_moving_average -- stock 0x40040F18 (172 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Moving average of width w = SYS16+0x662 (w >= 2) over the n = SYS+0x7BC samples of the channel buffer (SYS+0x7B4 + 4 ch), written in place at the window start: buf[i - w + 1] = (buf[i - w + 1] + ... + buf[i]) / w (signed).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void channel_moving_average(u32 ch)
{
    volatile unsigned short *buf = (volatile unsigned short *)SYS32(0x7B4 + 4 * ch);
    int n = (int)SYS32(0x7BC), w, i, acc = 0;

    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
    } else if (ch == 1) {
        if (SYS8(0x102) == 0)
            return;
    }

    w = SYS16(0x662);
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
