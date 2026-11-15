/*
 * channel_step_limiter -- stock 0x40040FC4 (204 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS_INFO+0x70D is set: smooths steps between neighbouring samples. Each pair moves towards each other by half their difference, clamped to +-lim, lim = SYS+0x1C0 x (15 at V/div index 12, the most sensitive range, else 5); 6 passes at index 12, else 3. The V/div index is SYS+0x41 for CH1, SYS+0x105 otherwise. Touches samples 0..n.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void channel_step_limiter(u32 ch)
{
    volatile unsigned short *buf = (volatile unsigned short *)SYS32(0x7B4 + 4 * ch);
    int n = (int)SYS32(0x7BC), lim, passes, j;
    u32 vd;
    if (SYS8(0x70D) == 0)
        return;
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
        vd = SYS8(0x41);
    } else {
        if (ch == 1 && SYS8(0x102) == 0)
            return;
        vd = SYS8(0x105);
    }
    lim = (int)(SYS32(0x1C0) * (vd == 12 ? 15u : 5u));
    passes = vd == 12 ? 6 : 3;
    do {
        for (j = 0; j < n; j++) {
            int b = buf[j + 1], a = buf[j], d = a - b;
            if (lim <= d) d = lim;
            if (d < -lim) d = -lim;
            buf[j] = a - d / 2;
            buf[j + 1] = b + d / 2;
        }
    } while (--passes);
}
