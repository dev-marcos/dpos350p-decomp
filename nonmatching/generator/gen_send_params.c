/*
 * gen_send_params -- stock 0x4004D1B4 (412 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Computes the generator FPGA parameters and sends them as jobs 0x14..0x18, then commits 0xB7: amplitude (corrected from 1 MHz up by the table at 0x400951E8, per MHz), divided by the float scale at +0x50 and +0x5C (or by +0x54 when the result is below it); offset; the floats at +0x70, +0x68, +0x6C; the step at +0x3C. Finally the period length (+0x60 / frequency, capped by +0x38) into G+0x2EF6. Skipped while the touch service is busy.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"
#include "generator.h"

void gen_send_params(void)
{
    volatile int *g = (volatile int *)(GEN_G + 0x2EFC);
    u32 a, r0, r1, s2;
    double d0, d1, d2;
    if (g_touch->serviced != 0)
        return;
    if (GEN_WAVE == 0 && g[0] >= 1000000) {
        int idx = (g[6] - g[0]) / 1000000;
        d0 = (double)g[1];
        d1 = (double)g[7];
        d1 = d0 * d1;
        d0 = (double)((volatile int *)0x400951E8u)[idx];
        d0 = d1 / d0;
        a = (u32)d0;
    } else {
        a = (u32)g[1];
    }
    d0 = (double)a;
    r1 = (u32)g[0x54 / 4];
    d1 = (double)*(volatile float *)&g[0x50 / 4];
    d0 = d0 / d1;
    s2 = (u32)g[0x5C / 4];
    d2 = d0 / (double)s2;
    r0 = (u32)d2;
    if (r0 < r1) {
        r0 = r1;
        d0 = d0 / (double)r1;
        s2 = (u32)d0;
    }
    g[0x3C / 4] = r0;
    g[0x34 / 4] = s2;
    fpga_param_updater(0x14, 4, g[2], g[2]);
    fpga_param_updater(0x15, 4, g[0x70 / 4], (int)*(volatile float *)&g[0x70 / 4]);
    fpga_param_updater(0x16, 4, g[0x68 / 4], (int)*(volatile float *)&g[0x68 / 4]);
    fpga_param_updater(0x17, 4, g[0x3C / 4], g[0x3C / 4]);
    fpga_param_updater(0x18, 4, g[0x6C / 4], (int)*(volatile float *)&g[0x6C / 4]);
    smec_commit(0xB7);
    {
        u32 fr = (u32)g[0], mx = (u32)g[0x38 / 4], v;
        v = fr != 0 ? (u32)g[0x60 / 4] / fr : mx;
        if (mx < (v & 0xFFFF))
            v = mx;
        G16(0x2EF6) = v;
    }
}
