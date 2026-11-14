# Battery monitor, front-end set-up (vertical scale, offsets, V/div labels, FPGA front-end selectors,
# calibration jobs), lane level measurements and the POST channel-multiplex check. exec'd by gen_platform.py


def h(addr, size, name, desc, body, src="frontend"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


# ---------------------------------------------------------------- battery
h(0x4004A2D0, 240, "battery_level_update",
  "Averaged LRADC code -> mV (float: code x slope + base + offset, truncated) -> battery level 0..10 in SYS_INFO+0x778 (thresholds 4150, 4080, 3970, 3900, 3840, 3790, 3760, 3730, 3690, 3620/3560 mV).", """
void battery_level_update(void)
{
    float t = (float)g_batt_avg;
    int mv, lv;
    t = g_batt_base + t * g_batt_slope;
    t = t + g_batt_offset;
    mv = (int)t;
    if (mv >= 4150) lv = 10;
    else if (mv >= 4080) lv = 9;
    else if (mv >= 3970) lv = 8;
    else if (mv >= 3900) lv = 7;
    else if (mv >= 3840) lv = 6;
    else if (mv >= 3790) lv = 5;
    else if (mv >= 3760) lv = 4;
    else if (mv >= 3730) lv = 3;
    else if (mv >= 3690) lv = 2;
    else if (mv >= 3620 || mv >= 3560) lv = 1;
    else lv = 0;
    SYS8(0x778) = lv;
}
""", src="power")
h(0x4004A3C0, 136, "battery_empty_check", "Battery level 0: stops timer 0, draws the empty-battery image at the screen centre, waits 3 s, powers off.", """
void battery_empty_check(void)
{
    u32 cx = SYS16(4) + (SYS16(8) >> 1);
    u32 cy = SYS16(6) + (SYS16(0x0A) >> 1);
    if (SYS8(0x778) == 0) {
        unsigned char *p = (unsigned char *)(0x42C2FE74u + 0xDD00);
        timer_stop(0);
        func_400342cc();
        func_40034134(cx + (signed char)p[0x194], cy + (signed char)p[0x195], *(u32 *)p, 0);
        delay_ms(3000);
        power_off();
    }
}
""", src="power")
h(0x4004A448, 148, "battery_service",
  "Charger status; LRADC readings are summed, and about once a second (>= 1001 ms, >= 3 readings) averaged into the battery level, then the empty-battery and auto power-off checks run.", """
void battery_service(void)
{
    charge_status_read();
    if (g_tick_count - g_batt_last >= 1001 && g_batt_count >= 3) {
        g_batt_avg = g_batt_sum / g_batt_count;
        g_batt_last = g_tick_count;
        g_batt_sum = 0;
        g_batt_count = 0;
        battery_level_update();
        battery_empty_check();
        auto_off_check();
        return;
    }
    g_batt_sum = lradc_data_read() + g_batt_sum;
    g_batt_count = g_batt_count + 1;
}
""", src="power")

# ---------------------------------------------------------------- front end
h(0x4004A50C, 4, "rate_table_init_entry", "Branch to rate_table_init.", """
void rate_table_init_entry(void)
{
    rate_table_init();
}
""")
h(0x4004A510, 64, "frontend_apply_all",
  "Applies every front-end setting: offsets, vertical scales, calibration selector, V/div labels, FPGA selectors 0x62/0x28/0x2B/0x2F/0x29/0x30/0x2C, channel mode.", """
void offset_clamp(void);
void vscale_update(void);
void vdiv_labels_update(void);
void fpga_write_sel_0x62(void);
void ch1_frontend_apply(void);
void ch2_frontend_apply(void);
void fpga_write_sel_0x2f(void);
void fpga_write_sel_0x29(void);
void fpga_write_sel_0x30(void);
void fpga_write_sel_0x2c(void);
void fpga_channel_mode_apply(void);

void frontend_apply_all(void)
{
    offset_clamp();
    vscale_update();
    retained_inside_current_sc1_3_t6();
    vdiv_labels_update();
    fpga_write_sel_0x62();
    ch1_frontend_apply();
    ch2_frontend_apply();
    fpga_write_sel_0x2f();
    fpga_write_sel_0x29();
    fpga_write_sel_0x30();
    fpga_write_sel_0x2c();
    fpga_channel_mode_apply();
}
""")
h(0x4004A550, 184, "rate_table_init",
  "u64 table at SYS_INFO+0xEF0: 20 M, 10 M, 5 M, 2 M, 1 M, 500 k, 200 k, 100 k, 50 k, 20 k, 10 k, 5 k, 2 k (indexed by the V/div index; values in uV per division before the probe factor).", """
void rate_table_init(void)
{
    static const u32 v[13] = { 20000000u, 10000000u, 5000000u, 2000000u, 1000000u, 500000u,
                               200000u, 100000u, 50000u, 20000u, 10000u, 5000u, 2000u };
    int i;
    for (i = 0; i < 13; i++) {
        SYS32(0xEF0 + 8 * i) = v[i];
        SYS32(0xEF4 + 8 * i) = 0;
    }
}
""")
h(0x4004A608, 444, "vscale_update",
  "Vertical scale per channel: table[V/div] x probe factor (x1, x10, x100 for probe 0/1/2) into SYS_INFO+0x50 (CH1) / +0x110 (CH2); a second value from table[SYS_INFO+0x1B9 - 1] into +0xD8 / +0x198. Other probe values leave them unchanged.", """
void vscale_update(void)
{
    u32 p1 = SYS8(0x40), p2 = SYS8(0x104);
    unsigned long long k, v198;
    if (p1 <= 2) {
        k = p1 == 0 ? 1 : p1 == 1 ? 10 : 100;
        SYS64(0x50) = SYS64(0xEF0 + 8 * SYS8(0x41)) * k;
        SYS64(0xD8) = SYS64(0xEE8 + 8 * SYS8(0x1B9)) * k;
    }
    v198 = SYS64(0x198);
    if (p2 > 2) {
        SYS32(0x198) = (u32)v198;
        SYS32(0x19C) = (u32)(v198 >> 32);
        return;
    }
    k = p2 == 0 ? 1 : p2 == 1 ? 10 : 100;
    SYS64(0x110) = SYS64(0xEF0 + 8 * SYS8(0x105)) * k;
    v198 = SYS64(0xEE8 + 8 * SYS8(0x1B9)) * k;
    SYS32(0x198) = (u32)v198;
    SYS32(0x19C) = (u32)(v198 >> 32);
}
""")
h(0x4004A7C4, 192, "offset_clamp",
  "Keeps the CH1/CH2 positions (SYS_INFO+0x60/+0x120) inside the screen: [m, height - m] with m = SYS_INFO+0x1AE, or [2m, height - 2m] when SYS_INFO+0x2AA is set.", """
void offset_clamp(void)
{
    int m = SYS16(0x1AE);
    int a = SYS32(0x60), b = SYS32(0x120), hgt = SYS16(0x0A);
    if (SYS8(0x2AA) == 0) {
        int hi = hgt - m;
        if (hi < a) a = hi;
        else if ((int)SYS16(0x1AE) >= a) a = SYS16(0x1AE);
        if (hi < b) b = hi;
        else if (!((int)SYS16(0x1AE) < b)) b = SYS16(0x1AE);
    } else {
        int lo = m * 2, hi = hgt - lo;
        if (hi < a) a = hi;
        else if (a < lo) a = lo;
        if (hi < b) b = hi;
        else if (b < lo) b = lo;
    }
    SYS32(0x60) = a;
    SYS32(0x120) = b;
}
""")
h(0x4004A884, 384, "vdiv_label",
  "V/div text for a channel (CH1 value SYS_INFO+0x50 -> text +0x70, CH2 +0x110 -> +0x130): the uV value divided by 1e9/1e6/1e3 gives up to three digits (no leading zeros) and KV, V or mV. Below 1000 uV the stock code uses the r2/r3 it was called with (passed through here).", """
void vdiv_label(u32 ch, u32 r1, int d, int unit)
{
    long long v;
    char *s;
    int hd, td, od, n = 0, any = 0;
    if (ch == 0) { v = (long long)SYS64(0x50); s = (char *)(0x40544E68u + 0x70); }
    else { v = (long long)SYS64(0x110); s = (char *)(0x40544E68u + 0x130); }
    if (v >= 1000000000LL) { d = (int)__aeabi_ldivmod(v, 1000000000LL); unit = 0; }
    else if (v >= 1000000LL) { d = (int)__aeabi_ldivmod(v, 1000000LL); unit = 1; }
    else if (v >= 1000LL) { d = (int)__aeabi_ldivmod(v, 1000LL); unit = 2; }
    hd = d / 100;
    td = (d - 100 * hd) / 10;
    od = d - 10 * (d / 10);
    if (hd != 0) { s[0] = hd + '0'; n = 1; any = 1; }
    if (any | td) { s[n++] = td + '0'; any = 1; }
    if (any | od) s[n++] = od + '0';
    if (unit == 0) { s[n] = 'K'; s[n + 1] = 'V'; s[n + 2] = 0; }
    else if (unit == 1) { s[n] = 'V'; s[n + 1] = 0; }
    else { s[n] = 'm'; s[n + 1] = 'V'; s[n + 2] = 0; }
}
""")
h(0x4004AA04, 32, "vdiv_labels_update", "Both channel labels.", """
void vdiv_labels_update(void)
{
    register int r2 asm("r2"), r3 asm("r3");
    asm volatile("" : "=r"(r2), "=r"(r3));        /* the stock code passes whatever r2/r3 hold */
    vdiv_label(0, 0, r2, r3);
    asm volatile("" : "=r"(r2), "=r"(r3));
    vdiv_label(1, 0, r2, r3);
}
""")
h(0x4004AA24, 76, "fpga_channel_mode_apply", "When SYS_INFO+0x100 changed (copy in +0xBCA): FPGA selector 0x2E <- value, then 50 ms.", """
void fpga_channel_mode_apply(void)
{
    if (SYS8(0xBCA) == SYS8(0x100))
        return;
    SYS8(0xBCA) = SYS8(0x100);
    fpga_selector_command_write(0x2E);
    fpga_data_byte_write(SYS8(0xBCA));
    delay_ms(50);
}
""")
h(0x4004AA70, 36, "fpga_write_sel_0x62", "FPGA selector 0x62 <- SYS_INFO+0x3A.", """
void fpga_write_sel_0x62(void)
{
    fpga_selector_command_write(0x62);
    fpga_data_byte_write(SYS8(0x3A));
}
""")
h(0x4004AA94, 44, "fpga_write_sel_0x2a", "FPGA selector 0x2A <- SYS_INFO+0x44 (16 bit, high byte first: CH1 sweep/DAC value).", """
void fpga_write_sel_0x2a(void)
{
    fpga_selector_command_write(0x2A);
    fpga_data_byte_write(SYS16(0x44) >> 8);
    fpga_data_byte_write(SYS16(0x44) & 0xFF);
}
""")
h(0x4004AAC0, 160, "ch1_frontend_apply",
  "CH1: V/div index to selector 0x28 when it changed (12 sent as 11, copy in +0xBC8), jobs 8 and 10 (calibration aux values), job 12 (position), commit 0xB5, selector 0x2A; 60 ms after a V/div change.", """
void fpga_write_sel_0x2a(void);

void ch1_frontend_apply(void)
{
    int changed = 0;
    if (SYS8(0xBC8) != SYS8(0x41)) {
        u32 v;
        SYS8(0xBC8) = SYS8(0x41);
        fpga_selector_command_write(0x28);
        v = SYS8(0xBC8);
        changed = 1;
        if (v == 12) v = 11;
        fpga_data_byte_write(v);
    }
    fpga_param_updater(8, 4, CAL_OBJ[0], CAL_OBJ[0]);
    fpga_param_updater(10, 4, CAL_OBJ[2], CAL_OBJ[2]);
    fpga_param_updater(12, 4, SYS32(0x60), SYS32(0x60));
    smec_commit(0xB5);
    fpga_write_sel_0x2a();
    if (changed)
        delay_ms(60);
}
""")
h(0x4004AB60, 36, "fpga_write_sel_0x2f", "FPGA selector 0x2F <- SYS_INFO+0x3F.", """
void fpga_write_sel_0x2f(void)
{
    fpga_selector_command_write(0x2F);
    fpga_data_byte_write(SYS8(0x3F));
}
""")
h(0x4004AB84, 36, "fpga_write_sel_0x29", "FPGA selector 0x29 <- SYS_INFO+0x47.", """
void fpga_write_sel_0x29(void)
{
    fpga_selector_command_write(0x29);
    fpga_data_byte_write(SYS8(0x47));
}
""")
h(0x4004ABA8, 36, "fpga_write_sel_0x2c", "FPGA selector 0x2C <- SYS_INFO+0x10B.", """
void fpga_write_sel_0x2c(void)
{
    fpga_selector_command_write(0x2C);
    fpga_data_byte_write(SYS8(0x10B));
}
""")
h(0x4004ABCC, 56, "fpga_write_sel_0x2d", "FPGA selector 0x2D <- SYS_INFO+0x108 (16 bit, high byte first: CH2 counterpart of 0x2A).", """
void fpga_write_sel_0x2d(void)
{
    fpga_selector_command_write(0x2D);
    fpga_data_byte_write(SYS16(0x108) >> 8);
    fpga_data_byte_write(SYS16(0x108) & 0xFF);
}
""")
h(0x4004AC04, 160, "ch2_frontend_apply",
  "CH2 counterpart of ch1_frontend_apply: selector 0x2B (copy in +0xBC9), jobs 9 and 11, job 13 (position +0x120), commit 0xB6, selector 0x2D.", """
void fpga_write_sel_0x2d(void);

void ch2_frontend_apply(void)
{
    int changed = 0;
    if (SYS8(0xBC9) != SYS8(0x105)) {
        u32 v;
        SYS8(0xBC9) = SYS8(0x105);
        fpga_selector_command_write(0x2B);
        v = SYS8(0xBC9);
        changed = 1;
        if (v == 12) v = 11;
        fpga_data_byte_write(v);
    }
    fpga_param_updater(9, 4, CAL_OBJ[1], CAL_OBJ[1]);
    fpga_param_updater(11, 4, CAL_OBJ[3], CAL_OBJ[3]);
    fpga_param_updater(13, 4, SYS32(0x120), SYS32(0x120));
    smec_commit(0xB6);
    fpga_write_sel_0x2d();
    if (changed)
        delay_ms(60);
}
""")
h(0x4004ACA4, 36, "fpga_write_sel_0x30", "FPGA selector 0x30 <- SYS_INFO+0x103.", """
void fpga_write_sel_0x30(void)
{
    fpga_selector_command_write(0x30);
    fpga_data_byte_write(SYS8(0x103));
}
""")

# ---------------------------------------------------------------- lane levels and the POST check
h(0x4004ACC8, 260, "lane_level_sum",
  "Sum over n raw samples of one lane of (sample x q - base), negative terms counted as 0, with q = gain[V/div] x SYS_INFO+0x26C / 1000 and base = q x SYS_INFO+0x1B8. Optionally (start) re-arms an acquisition of n x 1.2 samples first. Stock quirks kept: for ch > 1 the lane number is used as the gain, and the 64-bit difference takes base >> 31 (logical) as its high word (unreachable: base <= 4294967 x 255 < 2^31).", """
u32 lane_level_sum(u32 start, u32 lane, u32 ch, u32 n, unsigned char *buf)
{
    u32 gain, q, base, sum = 0, i;
    if (ch == 0) gain = SYS32(0x1C4 + 4 * SYS8(0x41));
    else if (ch == 1) gain = SYS32(0x214 + 4 * SYS8(0x105));
    else gain = lane;
    SYS32(0x63C) = n / 10;
    q = (gain * SYS32(0x26C)) / 1000;
    base = q * SYS8(0x1B8);
    if (start) {
        SYS32(0x30) = (n * 12) / 10;
        SYS32(0x34) = 0;
        fpga_sample_count_write((n * 12) / 10);
        acquisition_prepare();
        acq_wait_done();
    }
    func_400406a4(lane, n, buf);
    for (i = 0; i < n; i++) {
        unsigned long long p = (unsigned long long)buf[i] * q;
        u32 lo = (u32)p, hi = (u32)(p >> 32);
        u32 dlo = lo - base;
        u32 dhi = hi - (base >> 31) - (lo < base);
        if ((int)dhi < 0) dlo = 0;
        sum += dlo;
    }
    return sum;
}
""")
h(0x4004ADCC, 36, "lane_level_mean", "lane_level_sum / n.", """
u32 lane_level_mean(u32 start, u32 lane, u32 ch, u32 n, unsigned char *buf)
{
    return lane_level_sum(start, lane, ch, n, buf) / n;
}
""")
h(0x4004ADF0, 284, "channel_level_mean", "Sum of lane_level_sum over lanes 0..7, / n / 8.", """
u32 channel_level_mean(u32 start, u32 ch, u32 n, unsigned char *buf)
{
    u32 s0, s1, s2, s3, s4, s5, s6, s7;
    s0 = lane_level_sum(start, 0, ch, n, buf);
    s1 = lane_level_sum(start, 1, ch, n, buf) + s0;
    s2 = lane_level_sum(start, 2, ch, n, buf);
    s3 = lane_level_sum(start, 3, ch, n, buf);
    s4 = lane_level_sum(start, 4, ch, n, buf);
    s5 = lane_level_sum(start, 5, ch, n, buf);
    s6 = lane_level_sum(start, 6, ch, n, buf);
    s6 = s6 + (s2 + s1 + s3 + s4 + s5);
    s7 = lane_level_sum(start, 7, ch, n, buf);
    return ((s7 + s6) / n) >> 3;
}
""")
h(0x4004AF0C, 368, "post_channel_mux_check",
  "POST channel-multiplex test: both channels on at the given V/div, positions 0xAA then 0x1E; each time the measured mean position (SYS_INFO+0xB34 / +0xB50) must be within +-3 (+-6 at index 12). Returns 1 both pass, -2 CH1 only, -1 CH2 only, 0 neither.", """
int post_channel_mux_check(u32 vdiv)
{
    u32 tol = vdiv == 12 ? 6 : 3;
    unsigned char ok_hi1 = 0, ok_hi2 = 0, ok_lo1 = 0, ok_lo2 = 0;
    u32 a, m;
    SYS8(0x588) = 0x18; SYS8(0x41) = vdiv; SYS8(0x105) = vdiv;
    SYS8(0x3C) = 0; SYS8(0x3E) = 1; SYS8(0x40) = 0; SYS8(0x3F) = 0; SYS8(0x46) = 2;
    SYS8(0x102) = 1; SYS8(0x104) = 0; SYS8(0x103) = 0; SYS8(0x10A) = 2; SYS8(0x275) = 0; SYS8(0x660) = 0;
    SYS32(0x60) = 0xAA; SYS32(0x120) = 0xAA;
    func_4003e708(); delay_ms(100); acquisition_dispatcher();
    a = SYS32(0x60); m = SYS32(0xB34);
    if (a - tol < m && m < tol + a) ok_hi1 = 1;
    a = SYS32(0x120); m = SYS32(0xB50);
    if (a - tol < m && m < tol + a) ok_hi2 = 1;
    SYS32(0x60) = 0x1E; SYS32(0x120) = 0x1E;
    func_4003e708(); delay_ms(100); acquisition_dispatcher();
    a = SYS32(0x60); m = SYS32(0xB34);
    if (a - tol < m && m < tol + a) ok_lo1 = 1;
    a = SYS32(0x120); m = SYS32(0xB50);
    if (a - tol < m && m < tol + a) ok_lo2 = 1;
    {
        int c1 = (ok_lo1 & ok_hi1) != 0, c2 = (ok_lo2 & ok_hi2) != 0;
        if (c1 & c2) return 1;
        if (c1) return -2;
        if (c2) return -1;
        return 0;
    }
}
""", src="post")
