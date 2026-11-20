# Front-end offset calibration of one volts/div range (called by system_calibration) and the AUTO
# routine (vertical range and offset per channel, trigger source, time base), entered from
# buttons_05c_handler.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="calibration"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define TCH 0x42E68BB0u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define S8(a) (*(volatile signed char *)(a))
#define W32(a) (*(volatile u32 *)(a))
"""

wr(0x4004BFC8, 1404, "frontend_offset_calibrate",
   "Offset DAC calibration of volts/div range r (byte argument) for both channels: acq_defaults_reset, both channels on at range r (SYS+0x41 / +0x105) with "
   "the other channel fields cleared, SYS+0x588 = 0x18, SYS+0x3A = 1, touch byte 0x42E68BB0 + 0x16 = 0, outputs_apply, acquisition_dispatcher. Then two "
   "binary searches over the DAC range half SYS+0x1B6 .. SYS+0x1B4 (at most k steps, k = number of doublings of 1 up to the range width), CH1 through "
   "half SYS+0x44 + fpga_write_sel_0x2a, CH2 through SYS+0x108 + fpga_write_sel_0x2d, 100 ms settling, level = channel_level_mean(1, ch, 2000, "
   "0x429D3AB4) / word SYS+0x26C: first for a level in 420 .. 480, then in 20 .. 80 (SYS+0x1B2 flips the search direction). The two points give per "
   "channel gain = (dac_hi - dac_lo) x SYS+0x270 x SYS+0x26C / (mean_hi - mean_lo) (64-bit) and offset = SYS+0x270 x dac_lo - mean_lo x gain / SYS+0x26C, "
   "stored at 0x42E53A0C + 4r + 0x50 / + 0xA0 (gain CH1 / CH2) and + 0xF0 / + 0x140 (offset).",
   COM + """
#define K 0x429D3AB4u
#define DIV W32(SYS + 0x26C)
#define FLIP B8(SYS + 0x1B2)

void acq_defaults_reset(void);
void fpga_write_sel_0x2a(void);
void fpga_write_sel_0x2d(void);
void fpga_channel_mode_apply(void);
u32 channel_level_mean(u32 a, u32 ch, u32 n, u32 p);

static u32 mean(u32 ch)
{
    B8(SYS + 0x100) = ch + 1;
    fpga_channel_mode_apply();
    return channel_level_mean(1, ch, 2000, K);
}

/* one step of the search for channel ch: narrows lo / hi around mid, returns 1 when the level is inside lo_ok .. hi_ok */
static int step(u32 ch, int mid, int *lo, int *hi, int lo_ok, int hi_ok)
{
    int v = (int)(mean(ch) / DIV);

    if (v < lo_ok) {
        if (FLIP)
            *hi = mid;
        else
            *lo = mid;
        return 0;
    }
    if (v <= hi_ok)
        return 1;
    if (FLIP)
        *lo = mid;
    else
        *hi = mid;
    return 0;
}

static void search(int k, int lo_ok, int hi_ok)
{
    int lo1 = H16(SYS + 0x1B6), hi1 = H16(SYS + 0x1B4), lo2 = lo1, hi2 = hi1, m1 = 0, m2 = 0, d1 = 0, d2 = 0, it = 0;

    for (;;) {
        if (!d1) {
            m1 = (lo1 + hi1) / 2;
            H16(SYS + 0x44) = (unsigned short)m1;
            fpga_write_sel_0x2a();
        }
        if (!d2) {
            m2 = (lo2 + hi2) / 2;
            H16(SYS + 0x108) = (unsigned short)m2;
            fpga_write_sel_0x2d();
        }
        delay_ms(100);
        if (!d1)
            d1 = step(0, m1, &lo1, &hi1, lo_ok, hi_ok);
        if (!d2)
            d2 = step(1, m2, &lo2, &hi2, lo_ok, hi_ok);
        if (d1 && d2)
            return;
        if (++it >= k)
            return;
    }
}

static int gain(u32 dh, u32 dl, u32 vh, u32 vl)
{
    unsigned long long num = (unsigned long long)(long long)(int)(dh - dl) * W32(SYS + 0x270) * DIV;

    return (int)__aeabi_ldivmod((long long)num, (long long)(int)(vh - vl));
}

void frontend_offset_calibrate(u32 r)
{
    int k = 0, p = 1, w, g1, g2;
    u32 dh1, dh2, vh1, vh2, dl1, dl2, vl1, vl2, t;

    r &= 0xFF;
    acq_defaults_reset();
    B8(SYS + 0x3C) = 0;
    B8(SYS + 0x588) = 0x18;
    B8(SYS + 0x3E) = 1;
    B8(SYS + 0x41) = r;
    B8(SYS + 0x3F) = 0;
    B8(SYS + 0x40) = 0;
    B8(SYS + 0x46) = 0;
    B8(SYS + 0x102) = 1;
    B8(SYS + 0x105) = r;
    B8(SYS + 0x103) = 0;
    B8(SYS + 0x104) = 0;
    B8(SYS + 0x10A) = 0;
    B8(SYS + 0x275) = 0;
    B8(SYS + 0x660) = 0;
    B8(SYS + 0x3A) = 1;
    B8(TCH + 0x16) = 0;
    outputs_apply();
    acquisition_dispatcher();
    w = (int)H16(SYS + 0x1B4) - (int)H16(SYS + 0x1B6);
    do {
        p *= 2;
        k++;
    } while (w >= p);

    search(k, 0x1A4, 0x1E0);
    dh1 = H16(SYS + 0x44);
    dh2 = H16(SYS + 0x108);
    vh1 = mean(0);
    vh2 = mean(1);
    search(k, 0x14, 0x50);
    dl1 = H16(SYS + 0x44);
    dl2 = H16(SYS + 0x108);
    vl1 = mean(0);
    vl2 = mean(1);
    g1 = gain(dh1, dl1, vh1, vl1);
    g2 = gain(dh2, dl2, vh2, vl2);
    t = 0x42E53A0Cu + 4 * r;
    W32(t + 0x50) = (u32)g1;
    W32(t + 0xA0) = (u32)g2;
    W32(t + 0xF0) = W32(SYS + 0x270) * dl1 - (u32)((int)(vl1 * (u32)g1) / (int)DIV);
    W32(t + 0x140) = W32(SYS + 0x270) * dl2 - (u32)((int)(vl2 * (u32)g2) / (int)DIV);
}
""")

AUTO = COM + """
#define SAVED 0x409223F0u            /* backup copy of the settings block (state_backup_all) */
#define TBL 0x42C3D834u              /* message table: +0 text, +0x194 / +0x195 x / y offsets */

void fb_target_back(void);
void fb_present(void);
void ui_window_03_redraw(u32 a);
void ui_window_05_redraw(u32 a);
int percent_at_least(int x, int y, int pct);
void trigger_level_auto(void);

static void box(int cx, int cy)
{
    ui_window_03_redraw(0);
    ui_window_05_redraw(0);
    message_box_draw(cx + S8(TBL + 0x194), cy + S8(TBL + 0x195), W32(TBL), 0);
    fb_present();
}

/* moves offset *off of a channel whose trace spans up / dn pixels around it; returns 1 when it does not fit at pct % */
static int fit(int *off, u32 top, u32 bot, int pct)
{
    int h = H16(SYS + 0xA), o = *off, up = (int)W32(top) - o, dn = o - (int)W32(bot), a, b;

    a = percent_at_least(h - o - 30, up, pct);
    b = percent_at_least(o - 30, dn, pct);
    if (a == 0) {
        if (b == 0)
            return 1;
        o = (int)((u32)pct * (u32)(dn < 0 ? 0 : dn)) / 100 + 30;
        *off = o;
        return percent_at_least(h - o - 30, up, pct) == 0;
    }
    if (b != 0)
        return 0;
    o = h - (int)((u32)pct * (u32)(up < 0 ? 0 : up)) / 100 - 30;
    *off = o;
    return percent_at_least(o - 30, dn, pct) == 0;
}

/* final offset that brings the trace centre word c to row t, limited by the trace extent top / bot */
static void centre(u32 offw, u32 c, u32 t, u32 top, u32 bot)
{
    int h = H16(SYS + 0xA), n, d, lim;
    u32 v = W32(c);

    if (t < v) {
        lim = (int)W32(bot) - 30;
        if (lim < 0)
            lim = 0;
        d = (int)(v - t);
        if (d > lim)
            d = lim;
        n = (int)W32(offw) - d;
    } else {
        lim = h - 30 - (int)W32(top);
        if (lim < 0)
            lim = 0;
        d = (int)(t - v);
        if (d > lim)
            d = lim;
        n = d + (int)W32(offw);
    }
    if (h - 30 < n)
        n = h - 30;
    else if (n < 30)
        n = 30;
    W32(offw) = (u32)n;
}

void auto_set_from(u32 box_base)
{
    int cx = H16(box_base + 4) + (H16(box_base + 8) >> 1), cy = H16(box_base + 6) + (H16(box_base + 0xA) >> 1);
    int o1, o2, tb, tb_end, i, pct, d1, d2, f1 = 0, f2 = 0, any;
    u32 meas, t1 = 0, t2 = 0, h, a, b;

    state_backup_all();
    fb_target_back();
    meas = B8(SYS + 0x27C) ? 0x405459ACu : 0x40545990u;
    B8(SYS + 0x3C) = 0;
    B8(SYS + 0x588) = 0x1F;
    W32(SYS + 0x574) = 0;
    B8(SYS + 0x54F) = 1;
    B8(SYS + 0x41) = 0;
    W32(SYS + 0x570) = 0;
    W32(SYS + 0x60) = 0xFA;
    B8(SYS + 0x46) = 0;
    B8(SYS + 0x105) = 0;
    W32(SYS + 0x120) = 0xFA;
    B8(SYS + 0x10A) = 0;
    B8(SYS + 0x27A) = 0;
    B8(SYS + 0x275) = 0;
    B8(SYS + 0x665) = 0;
    B8(SYS + 0x660) = 0;
    B8(TCH + 0x15) = 0;
    B8(TCH + 0x16) = 0;
    tb = B8(SYS + 0x2A8);
    tb_end = B8(SYS + 0x2A9) - 1;
    o1 = (int)W32(SYS + 0x60);
    o2 = (int)W32(SYS + 0x120);
    if (B8(SYS + 0x1B0) == 1) {
        t1 = t2 = H16(SYS + 0xA) >> 1;
    } else if (B8(SYS + 0x1B0) == 2) {
        h = H16(SYS + 0xA);
        t2 = h >> 2;
        t1 = (h * 3) >> 2;
    }
    d1 = B8(SYS + 0x3E) == 0;
    d2 = B8(SYS + 0x102) == 0;
    for (i = 0; i < (int)B8(SYS + 0x1BA); i++) {
        u32 p = SYS + 0xEF0 + 8 * i;

        if (!d1) {
            B8(SYS + 0x41) = i;
            W32(SYS + 0x60) = (u32)o1;
        }
        if (!d2) {
            W32(SYS + 0x120) = (u32)o2;
            B8(SYS + 0x105) = i;
        }
        box(cx, cy);
        a = W32(p);
        b = W32(p + 4);
        pct = (int)__aeabi_ldivmod((long long)((((unsigned long long)b << 32) | a) * 100u),
                                   (long long)(((unsigned long long)W32(p + 12) << 32) | W32(p + 8)));
        if (!d1)
            d1 = fit(&o1, SYS + 0xB28, SYS + 0xB2C, pct);
        if (!d2)
            d2 = fit(&o2, SYS + 0xB44, SYS + 0xB48, pct);
        if (d1 && d2)
            break;
    }
    if (B8(SYS + 0x3E)) {
        if (W32(SYS + 0xB30) >= 0x33)
            f1 = 1;
        centre(SYS + 0x60, SYS + 0xB34, t1, SYS + 0xB28, SYS + 0xB2C);
    }
    if (B8(SYS + 0x102)) {
        if (W32(SYS + 0xB4C) >= 0x33)
            f2 = 1;
        centre(SYS + 0x120, SYS + 0xB50, t2, SYS + 0xB44, SYS + 0xB48);
    }
    outputs_apply();
    acquisition_dispatcher();
    any = f1 | f2;
    if (any) {
        if (f1 && !f2)
            B8(SYS + 0x27C) = 0;
        if (!f1 && f2)
            B8(SYS + 0x27C) = 1;
        B8(SYS + 0x275) = 1;
        B8(SYS + 0x274) = 1;
        H16(SYS + 0x27E) = (unsigned short)W32(meas + 0xC);
        W32(meas + 0x18) = (W32(meas + 8) << 1) / 3u;
        for (; tb <= tb_end; tb++) {
            B8(SYS + 0x588) = tb;
            box(cx, cy);
            if ((int)W32(meas + 0x10) >= 6)
                break;
        }
        trigger_level_auto();
        B8(SAVED + 0x27C) = B8(SYS + 0x27C);
        H16(SAVED + 0x27E) = H16(SYS + 0x27E);
        B8(SAVED + 0x588) = B8(SYS + 0x588);
    }
    B8(SAVED + 0x3C) = B8(SYS + 0x3C);
    B8(SAVED + 0x665) = B8(SYS + 0x665);
    B8(SAVED + 0x660) = B8(SYS + 0x660);
    B8(SAVED + 0x27A) = 0;
    B8(TCH + 0x16) = 1;
    if (any) {
        a = W32(SYS + 0x570);
        b = W32(SYS + 0x574);
        W32(SAVED + 0x570) = a;
        W32(SAVED + 0x574) = b;
    }
    if (B8(SYS + 0x3E) && f1) {
        B8(SAVED + 0x41) = B8(SYS + 0x41);
        W32(SAVED + 0x60) = W32(SYS + 0x60);
    }
    if (B8(SYS + 0x102) && f2) {
        B8(SAVED + 0x105) = B8(SYS + 0x105);
        W32(SAVED + 0x120) = W32(SYS + 0x120);
    }
    state_restore_all();
}
"""

wr(0x4004C7C8, 1856, "auto_set_from",
   "Body of auto_set, entered with r0 = the settings block 0x40544E68 (only its plot box +4 .. +0xA is read through it; it gives the centre of the "
   "message box). state_backup_all, fb_target_back, settings reset for the search (both offsets SYS+0x60 / +0x120 = 250, ranges SYS+0x41 / +0x105 = 0, "
   "time base SYS+0x588 = 0x1F, ...). Then for each volts/div range i < SYS+0x1BA (stopping a channel once its trace no longer fits): redraw "
   "with the message box, pct = 100 x int64 SYS+0xEF0 + 8i / int64 SYS+0xEF8 + 8i, and percent_at_least on the trace extent above / below the offset "
   "(SYS+0xB28 / +0xB2C for CH1, +0xB44 / +0xB48 for CH2) moves the offset. Each enabled channel whose amplitude word (SYS+0xB30 / +0xB4C) is at "
   "least 51 is 'found'; its offset moves the trace centre (SYS+0xB34 / +0xB50) to the row of the layout SYS+0x1B0 (1: half the height for both, 2: "
   "3/4 for CH1 and 1/4 for CH2), clamped to 30 .. height - 30. With a channel found the trigger source SYS+0x27C follows it (unchanged when both), "
   "the trigger level comes from the measurement block, and the time base steps SYS+0x2A8 .. SYS+0x2A9 - 1 until the period count word (+0x10) is at "
   "least 6; trigger_level_auto. The results are copied into the backup block 0x409223F0 before the tail call to state_restore_all. Layouts other "
   "than 1 / 2 leave the target row uninitialised in the stock code (OPEN; 0 here).",
   AUTO)

wr(0x4004C7C4, 4, "auto_set",
   "AUTO button (buttons_05c_handler): auto_set_from(0x40544E68) (the stock code loads r0 and falls through).",
   COM + """
void auto_set_from(u32 box_base);

void auto_set(void)
{
    auto_set_from(SYS);
}
""")
