/*
 * channel_step_limiter -- stock 0x40040FC4 (204 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS_INFO+0x70D is set: smooths steps between neighbouring samples. Each pair moves towards each other by half their difference, clamped to +-lim, lim = SYS+0x1C0 x (15 at V/div index 12, the most sensitive range, else 5); 6 passes at index 12, else 3. The V/div index is SYS+0x41 for CH1, SYS+0x105 otherwise. Touches samples 0..n.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void channel_step_limiter(u32 ch)
{
    volatile unsigned short *buf = (volatile unsigned short *)SYS32(0x7B4 + 4 * ch);
    int n = (int)SI(capture_samples), lim, passes, j;
    u32 vd;
    if (SI(step_limiter_on) == 0)
        return;
    if (ch == 0) {
        if (SI(ch1_enabled) == 0)
            return;
        vd = SI(ch1_vdiv_index);
    } else {
        if (ch == 1 && SI(ch2_enabled) == 0)
            return;
        vd = SI(ch2_vdiv_index);
    }
    lim = (int)(SI(acq_geometry_1c0) * (vd == 12 ? 15u : 5u));
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
