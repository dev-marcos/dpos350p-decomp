/*
 * lane_phase_measure8 -- stock 0x40064E0C (1152 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Phase error of the 8 interleaved lanes: three captures (fpga_pe_mode_select(0), acquisition_prepare, acq_wait_done, SYS+0x63C = 20, SYS16(0x640) = 5000, capture_banks_all, first_conversion_wrapper into 0x42C62AF4). Over samples 50 .. 39949 of 0x42C62B58, every pair crossing 16000 adds |s[i] - s[i+1]| to lane i % 8 (sums and counts carry over the three captures). With m_k = sum / count and r_k = m_k x 360 / (m_0 + .. + m_7), it writes m[0] = 0 and m[k] = 450 k - (int)(10 x (r_0 + .. + r_(k-1))) after each capture.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"
#include "system.h"
#include "util.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void first_conversion_wrapper_raw(u32 dst, u32 ch, u32 n) __asm__("first_conversion_wrapper");

#define LPH_LEVEL 0x3E80

/* adds |s[0] - s[1]| to lane i % nl when the pair crosses LPH_LEVEL */
static void lph_scan(const volatile unsigned short *s, int end, int nl, u32 *sum, u32 *cnt)
{
    int i;
    for (i = 50; i < end; i++, s++) {
        u32 a = s[0], b, d;
        if (a < LPH_LEVEL) {
            b = s[1];
            if (b <= LPH_LEVEL)
                continue;
        } else {
            if (a == LPH_LEVEL)
                continue;
            b = s[1];
            if (b >= LPH_LEVEL)
                continue;
        }
        d = a < b ? b - a : a - b;
        sum[i % nl] += d;
        cnt[i % nl]++;
    }
}

static void lph_capture_setup(void)
{
    SYS32(0x63C) = 20;
    SYS16(0x640) = 5000;
    trigger_pos_update();
    stream_geometry_fpga_selector_0x22_configuration();
}

void lane_phase_measure8(int *m)
{
    u32 sum[8], cnt[8];
    double q[8], s, r[8], c;
    int it, k;
    memset_w(sum, 0, sizeof(sum));
    memset_w(cnt, 0, sizeof(cnt));
    outputs_apply();
    fpga_sample_count_write(0x13EC);
    for (it = 0; it <= 2; it++) {
        fpga_pe_mode_select(0);
        acquisition_prepare();
        acq_wait_done();
        lph_capture_setup();
        capture_banks_all();
        first_conversion_wrapper_raw(0x42C62AF4u, 0, SYS16(0x640));
        lph_scan((const volatile unsigned short *)0x42C62B58u, 0x9C0E, 8, sum, cnt);
        for (k = 0; k < 8; k++)
            q[k] = (double)sum[k] / (double)cnt[k];
        s = q[0] + q[1] + q[2] + q[3] + q[4] + q[5] + q[6] + q[7];
        for (k = 0; k < 8; k++)
            r[k] = q[k] * 360.0 / s;
        m[0] = 0;
        c = r[0];
        m[1] = 450 - (int)(c * 10.0);
        for (k = 2; k < 8; k++) {
            c = c + r[k - 1];
            m[k] = 450 * k - (int)(c * 10.0);
        }
    }
}
