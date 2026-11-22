# Per-lane phase error measurements behind the generator self-test trim searches (batch 77):
# capture, find the samples where the trace crosses 16000, average |step| per interleaved lane and turn
# the averages into a phase error per lane in 0.1 degree.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wp(addr, size, name, desc, body, src="calibration"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


LPH = r"""
void fpga_pe_mode_select(u32 mode);
void stream_geometry_fpga_selector_0x22_configuration(void);

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
"""

wp(0x40064E0C, 1152, "lane_phase_measure8",
   "Phase error of the 8 interleaved lanes: three captures (fpga_pe_mode_select(0), acquisition_prepare, acq_wait_done, "
   "SYS+0x63C = 20, SYS16(0x640) = 5000, capture_banks_all, first_conversion_wrapper into 0x42C62AF4). Over samples "
   "50 .. 39949 of 0x42C62B58, every pair crossing 16000 adds |s[i] - s[i+1]| to lane i % 8 (sums and counts carry over "
   "the three captures). With m_k = sum / count and r_k = m_k x 360 / (m_0 + .. + m_7), it writes m[0] = 0 and "
   "m[k] = 450 k - (int)(10 x (r_0 + .. + r_(k-1))) after each capture.", LPH + r"""
void capture_banks_all(void);
void first_conversion_wrapper(u32 dst, u32 ch, u32 n);

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
        first_conversion_wrapper(0x42C62AF4u, 0, SYS16(0x640));
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
""")

wp(0x4006529C, 1312, "lane_phase_measure4x2",
   "Phase error of two groups of 4 lanes: five rounds of two captures (banks 0..3 unpacked into 0x42C62AF4 by "
   "bank_unpack_0_3_n, banks 4..7 into 0x42C7B964 by second_conversion_wrapper). Over samples 50 .. 19949 of each "
   "buffer (+0x64) every pair crossing 16000 adds |step| to lane i % 4. With r_k = m_k x 360 / (m_0 + .. + m_3): "
   "m[0] = 0, m[k] = 900 k - (int)(10 x (r_0 + .. + r_(k-1))), for both groups (m1, m2).", LPH + r"""
void capture_banks_0_3(void);
void capture_banks_4_7(void);
void bank_unpack_0_3_n(u32 dst, u32 ch, u32 n);
void second_conversion_wrapper(u32 dst, u32 ch, u32 n);

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
        bank_unpack_0_3_n(0x42C62AF4u, 0, SYS16(0x640));
        lph_capture_setup();
        capture_banks_4_7();
        second_conversion_wrapper(0x42C7B964u, 1, SYS16(0x640));
        lph_scan((const volatile unsigned short *)0x42C62B58u, 0x4DEE, 4, sum1, cnt1);
        lph_scan((const volatile unsigned short *)0x42C7B9C8u, 0x4DEE, 4, sum2, cnt2);
        lph_out4(m1, sum1, cnt1);
        lph_out4(m2, sum2, cnt2);
    }
}
""")
