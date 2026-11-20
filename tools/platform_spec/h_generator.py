# Signal generator (settings limits, sample tables, FPGA jobs 0x14..0x18), the calibration
# coefficient selector and the system calibration routine. exec'd by gen_platform.py


def k(addr, size, name, desc, body, src="generator"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


# ---------------------------------------------------------------- calibration
k(0x4004C544, 368, "retained_inside_current_sc1_3_t6",
  "Calibration coefficient selector: aux values for the CH1/CH2 V/div (object+0..+0xC) and one matrix row (min(SYS+0x588, SYS+0x6ED) - SYS+0x6EC) into the active coefficients A/B (object+0x10..+0x4C): columns 16..31 when SYS_INFO+0x100 is 0, else 0..15. (name: Hack STOCK_LIKE_MINIMAL_RUNTIME.md working label)", """
void retained_inside_current_sc1_3_t6(void)
{
    volatile u32 *o = CAL_OBJ;
    u32 i1 = SYS8(0x41), i2, a, b, c, row, base, j;
    volatile u32 *p;
    o[0] = o[0x50 / 4 + i1];
    o[2] = o[0xF0 / 4 + i1];
    i2 = SYS8(0x105);
    o[1] = o[0xA0 / 4 + i2];
    o[3] = o[0x140 / 4 + i2];
    a = SYS8(0x588);
    b = SYS8(0x6ED);
    c = SYS8(0x6EC);
    row = (b >= a) ? a - c : b - c;
    p = (volatile u32 *)((u32)CAL_OBJ + row * 4);
    base = SYS8(0x100) == 0 ? 16 : 0;
    for (j = 0; j < 16; j++)
        o[4 + j] = p[(0x190 + (base + j) * 0xC8) / 4];
}
""", src="calibration; name: Hack")
k(0x4004C6B4, 212, "system_calibration",
  "System calibration (factory): clears the calibration object, calibrates each row, fills the aux arrays above the calibrated rows with the last value, runs the channel-multiplex check for V/div 0..10 and saves the calibration only when all pass. Returns the last check result.", """
int system_calibration(void)
{
    u32 i, r;
    int res;
    state_backup_all();
    memset_w((void *)CAL_OBJ, 0, 0x1A90);
    for (i = 0; i < SYS8(0x1B9); )
        frontend_offset_calibrate(i++ & 0xFF);
    {
        u32 b = SYS8(0x1B9), a = SYS8(0x1BA);
        if (b < a) {
            volatile u32 *src = (volatile u32 *)((u32)CAL_OBJ + b * 4);
            volatile u32 *dst = (volatile u32 *)((u32)CAL_OBJ + b * 4 + 0x50);
            u32 n = a - b;
            do {
                dst[0] = src[0x4C / 4];
                dst[0xA0 / 4] = src[0xEC / 4];
                dst[0x50 / 4] = src[0x9C / 4];
                dst[0xF0 / 4] = src[0x13C / 4];
                dst++;
            } while (--n);
        }
    }
    func_4004b0a0();
    for (r = 0; ; ) {
        res = post_channel_mux_check(r & 0xFF);
        if (res != 1)
            break;
        if (++r >= 11) {
            calibration_save_wrapper();
            break;
        }
    }
    state_restore_all();
    g_touch->enabled = 1;
    return res;
}
""", src="calibration")
k(0x4004C794, 48, "percent_at_least", "1 when max(v, 0) x 100 / total >= min_pct or total <= 0, else 0.", """
u32 percent_at_least(int v, int total, int min_pct)
{
    if (v < 0)
        v = 0;
    if (total <= 0)
        return 1;
    if ((v * 100) / total < min_pct)
        return 0;
    return 1;
}
""", src="calibration")

# ---------------------------------------------------------------- generator
k(0x4004CF34, 352, "gen_limits",
  "Clamps the GEN settings (G+0x2EFC): amplitude to [0, max], offset to +-limit and offset +- amplitude/2 into the output range, waveform index, duty to [1, max], frequency to the waveform's maximum (50/10/5/3 MHz; above index 14 the stock code uses the r2/r3 it was called with). Skipped while the touch service is busy.", """
void gen_limits(u32 r0, u32 r1, u32 lo, u32 hi)
{
    volatile int *g = (volatile int *)(GEN_G + 0x2EFC);
    int off, amp, lim, half, nw, d, fr;
    if (g_touch->serviced != 0)
        return;
    off = g[2];
    amp = g[1];
    if (g[7] < amp) g[1] = g[7];
    else if (amp < 0) g[1] = 0;
    lim = g[8];
    if (lim < off) off = lim;
    else if (off < -lim) off = -lim;
    amp = g[1];
    half = (amp + (int)((u32)amp >> 31)) >> 1;
    if (g[10] < off + half) off = g[10] - half;
    else if (off - half < g[11]) off = g[11] + half;
    nw = g[5];
    if (nw < (int)GEN_WAVE) GEN_WAVE = nw;
    d = g[3];
    if (g[9] < d) g[3] = g[9];
    else if (d <= 0) g[3] = 1;
    switch (GEN_WAVE) {
    case 0: case 9: lo = 50000000; hi = 0; break;
    case 1: case 6: case 7: lo = 10000000; hi = 0; break;
    case 2: case 3: case 4: case 5: case 11: case 12: case 13: lo = 5000000; hi = 0; break;
    case 8: case 10: case 14: lo = 3000000; hi = 0; break;
    default: break;
    }
    fr = g[0];
    if ((long long)(((unsigned long long)hi << 32) | lo) < (long long)fr) g[0] = lo;
    else if (fr < 0) g[0] = 0;
    g[2] = off;
}
""")
k(0x4004D094, 8, "gen_table_repeat_default", "gen_table_repeat on the table descriptor at G+0xFAE. The stock code falls through into it (the analyser split the entry).", """
void gen_table_repeat_default(void)
{
    register u32 r1 asm("r1");
    asm volatile("" : "=r"(r1));
    gen_table_repeat(0xFAE, r1, g_touch);
}
""")
k(0x4004D09C, 144, "gen_table_repeat",
  "Table descriptor at G+off (length, top, -, -, count at +6): one period = length / count samples; the period is inverted (top - sample) and repeated count times over the buffer G+0xE. Skipped while the touch service is busy.", """
void gen_table_repeat(u32 off, u32 r1, volatile touch_t *t)
{
    volatile unsigned short *e = (volatile unsigned short *)(GEN_G + off);
    int per = (int)e[0] / (int)e[3];
    u32 j;
    if (t->serviced != 0)
        return;
    if (per != 0) {
        unsigned short *p = GEN_BUF;
        int n = per;
        do {
            *p = e[1] - *p;
            p++;
        } while (--n);
    }
    if (e[3] > 1) {
        j = 1;
        do {
            unsigned short *dst = GEN_BUF + j * per;
            j++;
            copy_u16(dst, GEN_BUF, per);
        } while (j < e[3]);
    }
}
""")
k(0x4004D12C, 56, "gen_load_samples", "Sample count from SYS_INFO+0x3DA53C into G+0x2EF8, then that many samples from SYS_INFO+0x3DA548 into G+0x1F56.", """
void gen_load_samples(void)
{
    G16(0x2EF8) = SYS32(0x3DA53C);
    copy_u16((unsigned short *)(GEN_G + 0x1F56), (const unsigned short *)(0x40544E68u + 0x3DA548), G16(0x2EF8));
}
""")
k(0x4004D164, 40, "Gen_Init",
  "Reads the generator files (0x40036B00, 0x40036BDC), applies the value at 0x4091F396, loads the samples, then continues into gen_update (the stock code falls through). (name: Hack FPGA.md)", """
void gen_update(void);

void Gen_Init(void)
{
    list_c_index_load();
    list_c_data_load();
    list_c_record_select(*(volatile unsigned short *)0x4091F396u);
    gen_load_samples();
    gen_update();
}
""", src="generator; name: Hack FPGA.md")
k(0x4004D18C, 40, "gen_update", "Generator update: 0x4004EEEC, limits, FPGA jobs, table build (0x4004D880), 0x4004ED18, table repeat.", """
void gen_update(void)
{
    register u32 r0 asm("r0"), r1 asm("r1"), r2 asm("r2"), r3 asm("r3");
    fpga_cmd63_write();
    asm volatile("" : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3));   /* gen_limits gets whatever r0..r3 hold, as in the stock code */
    gen_limits(r0, r1, r2, r3);
    gen_send_params();
    func_4004d880();
    gen_fpga_settings_write();
    gen_table_repeat_default();
}
""")
k(0x4004D1B4, 412, "gen_send_params",
  "Computes the generator FPGA parameters and sends them as jobs 0x14..0x18, then commits 0xB7: amplitude (corrected from 1 MHz up by the table at 0x400951E8, per MHz), divided by the float scale at +0x50 and +0x5C (or by +0x54 when the result is below it); offset; the floats at +0x70, +0x68, +0x6C; the step at +0x3C. Finally the period length (+0x60 / frequency, capped by +0x38) into G+0x2EF6. Skipped while the touch service is busy.", """
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
""")
k(0x4004D350, 64, "gen_decimate", "Keeps every n-th sample of the table G+0xE (count G+0x2EF8 / n), n > 1.", """
void gen_decimate(int n)
{
    int m;
    unsigned short *s, *d;
    if (n <= 1)
        return;
    m = (int)G16(0x2EF8) / n;
    if (m < 1)
        return;
    s = GEN_BUF;
    d = GEN_BUF;
    do {
        *d++ = *s;
        s += n;
    } while (--m);
}
""")
k(0x4004D390, 268, "gen_normalize",
  "Copies the loaded samples (G+0x1F56) to the table, inverts them (max - v) and scales them to span 2 x (G+0xFB0 / 2) in double precision (truncated).", """
void gen_normalize(void)
{
    u32 half = G16(0xFB0) >> 1;
    int mx = 0, mn = 0x7FFFFFFF;
    u32 n, i;
    double sc, hd;
    copy_u16(GEN_BUF, (const unsigned short *)(GEN_G + 0x1F56), G16(0x2EF8));
    n = G16(0x2EF8);
    for (i = 0; i < n; i++) {
        int v = GEN_BUF[i];
        if (v >= mx) mx = v;
        if (v <= mn) mn = v;
    }
    if (n != 0) {
        i = 0;
        do {
            GEN_BUF[i] = mx - GEN_BUF[i];
            i++;
        } while ((int)i < (int)G16(0x2EF8));
    }
    sc = (double)(int)half * 2.0;
    sc = sc / (double)(mx - mn);
    hd = (double)(int)half;
    if (G16(0x2EF8) != 0) {
        i = 0;
        do {
            double t = hd;
            t = t + (double)GEN_BUF[i] * sc;
            t = t - hd;
            GEN_BUF[i] = (int)t;
            i++;
        } while ((int)i < (int)G16(0x2EF8));
    }
}
""")
k(0x4004D49C, 304, "gen_resample",
  "Resamples one period of the table: period = length / count (descriptor G+0xFAE); step = (u64)(period x 1.02) x SYS_INFO+0x554 x m / total with m = total / period + 1; decimates by m, resamples into the work buffer 0x41648008 with the nearest-neighbour mapper, copies back, repeats the end samples and clamps to G+0xFB0.", """
void gen_resample(void)
{
    u32 per = (int)G16(0xFAE) / (int)G16(0xFAE + 6);
    u32 tot = G16(0x2EF8);
    u32 m = tot / per + 1;
    unsigned long long x = D2ULZ((double)per * 1.02);
    u32 step;
    unsigned short *w = (unsigned short *)(0x416431E8u + 0x4E20);
    u32 i;
    x = x * SYS32(0x554);
    x = x * m;
    step = (u32)__aeabi_uldivmod(x, tot);
    gen_decimate(m & 0xFF);
    {
        u32 q = tot / m;
        copy_u16(w - (q >> 1), GEN_BUF, q);
    }
    simple_integer_index_mapping_nearest_neighbour(0x416431E8u, 10000, per & 0xFFFF, step, 0, 0);
    copy_u16(GEN_BUF, w - (per >> 1), per);
    GEN_BUF[0] = GEN_BUF[1];
    GEN_BUF[per - 1] = GEN_BUF[per - 2];
    for (i = 0; i < per; i++) {
        unsigned short lim = G16(0xFB0), v = GEN_BUF[i];
        GEN_BUF[i] = lim > v ? v : lim;
    }
}
""")
k(0x4004D5CC, 76, "gen_decimate2", "gen_decimate for the second table (G+0xFB6).", """
void gen_decimate2(int n)
{
    int m;
    unsigned short *s, *d;
    if (n <= 1)
        return;
    m = (int)G16(0x2EF8) / n;
    if (m < 1)
        return;
    s = GEN_BUF2;
    d = GEN_BUF2;
    do {
        *d++ = *s;
        s += n;
    } while (--m);
}
""")
k(0x4004D618, 272, "gen_normalize2",
  "Second table: copy from G+0x1F56, invert, scale to span 2 x amplitude (G+0x2F30) and shift by G+0x2F48 - amplitude, in double precision (truncated).", """
void gen_normalize2(void)
{
    int mx = 0, mn = 0x7FFFFFFF;
    u32 n, i;
    double sc;
    copy_u16(GEN_BUF2, (const unsigned short *)(GEN_G + 0x1F56), G16(0x2EF8));
    n = G16(0x2EF8);
    for (i = 0; i < n; i++) {
        int v = GEN_BUF2[i];
        if (v >= mx) mx = v;
        if (v <= mn) mn = v;
    }
    if (n != 0) {
        i = 0;
        do {
            GEN_BUF2[i] = mx - GEN_BUF2[i];
            i++;
        } while ((int)i < (int)G16(0x2EF8));
    }
    sc = (double)G32(0x2F30) * 2.0;
    sc = sc / (double)(mx - mn);
    if (G16(0x2EF8) != 0) {
        i = 0;
        do {
            double t = (double)G32(0x2F48);
            t = t + (double)GEN_BUF2[i] * sc;
            t = t - (double)GS32(0x2F30);
            GEN_BUF2[i] = (int)t;
            i++;
        } while ((int)i < (int)G16(0x2EF8));
    }
}
""")
k(0x4004D730, 280, "gen_resample2",
  "gen_resample for the second table: period from G+0x2EF6, then each sample capped at 2 x G+0x2F58.", """
void gen_resample2(void)
{
    u32 per = G16(0x2EF6);
    u32 tot = G16(0x2EF8);
    u32 m = tot / per + 1;
    unsigned long long x = D2ULZ((double)per * 1.02);
    u32 step, i;
    unsigned short *w = (unsigned short *)(0x416431E8u + 0x4E20);
    x = x * SYS32(0x554);
    x = x * m;
    step = (u32)__aeabi_uldivmod(x, tot);
    gen_decimate2(m & 0xFF);
    {
        u32 q = tot / m;
        copy_u16(w - (q >> 1), GEN_BUF2, q);
    }
    simple_integer_index_mapping_nearest_neighbour(0x416431E8u, 10000, per, step, 0, 0);
    copy_u16(GEN_BUF2, w - (per >> 1), per);
    for (i = 0; i < per; i++) {
        u32 lim = G32(0x2F58);
        if (GEN_BUF2[i] > (lim << 1))
            GEN_BUF2[i] = lim << 1;
    }
}
""")
