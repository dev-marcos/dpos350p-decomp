/*
 * gen_selftest_50898 -- stock 0x40050898 (788 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Self-test with five generator / capture settings (DERIVED from what it writes): acq_defaults_reset, channels on (SYS+0x3E = 1, +0x102 = 1), coupling bytes +0x46 / +0x10A = 2, SYS+0x660 = 5, touch byte +0x16 = 0. For each step i: waveform byte 0x42E3F984+0x2EFA = 1, frequency +0x2EFC = 25 / 100 / 1000 / 10000 / 10000, amplitude +0x2F00 = 1200 (3000 in the last step), offset +0x2F04 = 0, time-base SYS+0x588 = 0x20 / 0x1E / 0x1B / 0x18 / 0x18, ranges SYS+0x41 = SYS+0x105 = 6 (5 in the last step); outputs_apply, gen_update, acquisition_dispatcher. Per channel the peak-to-peak of the capture window (u16 samples at 0x42AC7CF4 / +0x62250 from 0x2E68 + 5 x step for 0x924 - 10 x step samples, step = SYS+0x558) passes when it is at most 10 x word SYS+0x1C0. The AND of the five results goes to byte 0x42E68AE0+0x35 (A) and +0x36 (B), both together to +0x34.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "generator.h"
#include "selftest.h"
#include "system.h"
#include "sys_info.h"
#include "state/factory_block.h"
#include "state/gen_block.h"
#include "shared/access.h"

#define SYS 0x40544E68u

void gen_selftest_50898(void)
{
    static const u32 freq[5] = {25, 100, 1000, 10000, 10000};
    static const u32 amp[5] = {1200, 1200, 1200, 1200, 3000};
    static const unsigned char tb[5] = {0x20, 0x1E, 0x1B, 0x18, 0x18};
    static const unsigned char rng[5] = {6, 6, 6, 6, 5};
    u32 i, j, step, start, n, lim, ok_a = 1, ok_b = 1;
    int mx, mn, v;
    volatile unsigned short *s;
    acq_defaults_reset();
    SI(ch1_enabled) = 1;
    SI(ch2_enabled) = 1;
    SI(f_046) = 2;
    SI(f_10a) = 2;
    SI(f_660) = 5;
    g_touch->enabled = 0;
    step = SI(f_558);
    start = 0x2E68 + 5 * step;
    n = 0x924 - 10 * step;
    for (i = 0; i < 5; i++) {
        GENS(waveform) = 1;
        GENS(frequency) = freq[i];
        GENS(amplitude) = amp[i];
        GENS(offset) = 0;
        SI(timebase_index) = tb[i];
        SI(ch1_vdiv_index) = rng[i];
        SI(ch2_vdiv_index) = rng[i];
        outputs_apply();
        gen_update();
        acquisition_dispatcher();
        mx = 0;
        mn = 0x0FFFFFFF;
        if ((int)start < (int)(start + n)) {
            s = (volatile unsigned short *)(0x42AC7CF4u + 2 * start);
            for (j = 0; j < n; j++) {
                v = s[j];
                if (v >= mx)
                    mx = v;
                if (v <= mn)
                    mn = v;
            }
        }
        lim = SI(acq_geometry_1c0) * 10;
        ok_a &= lim >= (u32)(mx - mn) ? 1u : 0u;
        mx = 0;
        mn = 0x0FFFFFFF;
        if ((int)start < (int)(start + n)) {
            s = (volatile unsigned short *)(0x42AC7CF4u + 0x62250u + 2 * start);
            for (j = 0; j < n; j++) {
                v = s[j];
                if (v >= mx)
                    mx = v;
                if (v < mn)
                    mn = v;
            }
        }
        lim = SI(acq_geometry_1c0) * 10;
        ok_b &= lim >= (u32)(mx - mn) ? 1u : 0u;
    }
    FACT(window_a_ok) = (unsigned char)ok_a;
    FACT(window_b_ok) = (unsigned char)ok_b;
    FACT(f_034) = (unsigned char)(ok_a & ok_b);
}
