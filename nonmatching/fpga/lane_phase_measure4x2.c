/*
 * lane_phase_measure4x2 -- stock 0x4006529C (1312 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Phase error of two groups of 4 lanes: five rounds of two captures (banks 0..3 unpacked into 0x42C62AF4 by bank_unpack_0_3_n, banks 4..7 into 0x42C7B964 by second_conversion_wrapper). Over samples 50 .. 19949 of each buffer (+0x64) every pair crossing 16000 adds |step| to lane i % 4. With r_k = m_k x 360 / (m_0 + .. + m_3): m[0] = 0, m[k] = 900 k - (int)(10 x (r_0 + .. + r_(k-1))), for both groups (m1, m2).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"
#include "system.h"
#include "sys_info.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void bank_unpack_0_3_n_raw(u32 dst, u32 ch, u32 n) __asm__("bank_unpack_0_3_n");
void second_conversion_wrapper_raw(u32 dst, u32 ch, u32 n) __asm__("second_conversion_wrapper");

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
    SI(runtime_quotient) = 20;
    SI(f_640) = 5000;
    trigger_pos_update();
    stream_geometry_fpga_selector_0x22_configuration();
}

static void lph_out4(int *m, const u32 *sum, const u32 *cnt)
{
    double q[4], s, r[4], c;
    int k;
    for (k = 0; k < 4; k++)
        q[k] = (double)sum[k] / (double)cnt[k];
    s = q[0] + q[1] + q[2] + q[3];
    for (k = 0; k < 4; k++)
        r[k] = q[k] * 360.0 / s;
    m[0] = 0;
    c = r[0];
    m[1] = 900 - (int)(c * 10.0);
    c = c + r[1];
    m[2] = 1800 - (int)(c * 10.0);
    c = c + r[2];
    m[3] = 2700 - (int)(c * 10.0);
}

void lane_phase_measure4x2(int *m1, int *m2)
{
    u32 sum1[4] = {0}, cnt1[4] = {0}, sum2[4] = {0}, cnt2[4] = {0};
    int it;
    outputs_apply();
    fpga_sample_count_write(0x13EC);
    for (it = 0; it <= 4; it++) {
        fpga_pe_mode_select(0);
        acquisition_prepare();
        acq_wait_done();
        lph_capture_setup();
        capture_banks_0_3();
        bank_unpack_0_3_n_raw(0x42C62AF4u, 0, SI(f_640));
        lph_capture_setup();
        capture_banks_4_7();
        second_conversion_wrapper_raw(0x42C7B964u, 1, SI(f_640));
        lph_scan((const volatile unsigned short *)0x42C62B58u, 0x4DEE, 4, sum1, cnt1);
        lph_scan((const volatile unsigned short *)0x42C7B9C8u, 0x4DEE, 4, sum2, cnt2);
        lph_out4(m1, sum1, cnt1);
        lph_out4(m2, sum2, cnt2);
    }
}
