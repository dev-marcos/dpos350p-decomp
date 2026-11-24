/*
 * auto_set_from -- stock 0x4004C7C8 (1856 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Body of auto_set, entered with r0 = the settings block 0x40544E68 (only its plot box +4 .. +0xA is read through it; it gives the centre of the message box). state_backup_all, fb_target_back, settings reset for the search (both offsets SYS+0x60 / +0x120 = 250, ranges SYS+0x41 / +0x105 = 0, time base SYS+0x588 = 0x1F, ...). Then for each volts/div range i < SYS+0x1BA (stopping a channel once its trace no longer fits): redraw with the message box, pct = 100 x int64 SYS+0xEF0 + 8i / int64 SYS+0xEF8 + 8i, and percent_at_least on the trace extent above / below the offset (SYS+0xB28 / +0xB2C for CH1, +0xB44 / +0xB48 for CH2) moves the offset. Each enabled channel whose amplitude word (SYS+0xB30 / +0xB4C) is at least 51 is 'found'; its offset moves the trace centre (SYS+0xB34 / +0xB50) to the row of the layout SYS+0x1B0 (1: half the height for both, 2: 3/4 for CH1 and 1/4 for CH2), clamped to 30 .. height - 30. With a channel found the trigger source SYS+0x27C follows it (unchanged when both), the trigger level comes from the measurement block, and the time base steps SYS+0x2A8 .. SYS+0x2A9 - 1 until the period count word (+0x10) is at least 6; trigger_level_auto. The results are copied into the backup block 0x409223F0 before the tail call to state_restore_all. Layouts other than 1 / 2 leave the target row uninitialised in the stock code (OPEN; 0 here).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "frontend.h"
#include "system.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "shared/access.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_03_redraw_raw(u32 a) __asm__("ui_window_03_redraw");
void ui_window_05_redraw_raw(u32 a) __asm__("ui_window_05_redraw");

#define SYS 0x40544E68u
#define S8(a) (*(volatile signed char *)(a))

#define SAVED 0x409223F0u            /* backup copy of the settings block (state_backup_all) */
#define TBL 0x42C3D834u              /* message table: +0 text, +0x194 / +0x195 x / y offsets */

static void box(int cx, int cy)
{
    ui_window_03_redraw_raw(0);
    ui_window_05_redraw_raw(0);
    message_box_draw(cx + S8(TBL + 0x194), cy + S8(TBL + 0x195), W32(TBL), 0);
    fb_present();
}

/* moves offset *off of a channel whose trace spans up / dn pixels around it; returns 1 when it does not fit at pct % */
static int fit(int *off, u32 top, u32 bot, int pct)
{
    int h = SI(plot_height), o = *off, up = (int)W32(top) - o, dn = o - (int)W32(bot), a, b;

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
    int h = SI(plot_height), n, d, lim;
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
    meas = SI(trigger_channel) ? 0x405459ACu : 0x40545990u;
    SI(acq_transition_flag) = 0;
    SI(timebase_index) = 0x1F;
    W32(SYS + 0x574) = 0;
    SI(f_54f) = 1;
    SI(ch1_vdiv_index) = 0;
    W32(SYS + 0x570) = 0;
    SI(ch1_offset) = 0xFA;
    SI(f_046) = 0;
    SI(ch2_vdiv_index) = 0;
    SI(ch2_offset) = 0xFA;
    SI(f_10a) = 0;
    SI(trigger_mode) = 0;
    SI(f_275) = 0;
    SI(f_665) = 0;
    SI(hires_option) = 0;
    g_touch->busy2 = 0;
    g_touch->enabled = 0;
    tb = SI(timebase_index_min);
    tb_end = SI(slow_timebase_index) - 1;
    o1 = (int)SI(ch1_offset);
    o2 = (int)SI(ch2_offset);
    if (SI(sample_divisor) == 1) {
        t1 = t2 = SI(plot_height) >> 1;
    } else if (SI(sample_divisor) == 2) {
        h = SI(plot_height);
        t2 = h >> 2;
        t1 = (h * 3) >> 2;
    }
    d1 = SI(ch1_enabled) == 0;
    d2 = SI(ch2_enabled) == 0;
    for (i = 0; i < (int)SI(f_1ba); i++) {
        u32 p = SYS + 0xEF0 + 8 * i;

        if (!d1) {
            SI(ch1_vdiv_index) = i;
            SI(ch1_offset) = (u32)o1;
        }
        if (!d2) {
            SI(ch2_offset) = (u32)o2;
            SI(ch2_vdiv_index) = i;
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
    if (SI(ch1_enabled)) {
        if (SI(ch1_amplitude) >= 0x33)
            f1 = 1;
        centre(SYS + 0x60, SYS + 0xB34, t1, SYS + 0xB28, SYS + 0xB2C);
    }
    if (SI(ch2_enabled)) {
        if (SI(ch2_amplitude) >= 0x33)
            f2 = 1;
        centre(SYS + 0x120, SYS + 0xB50, t2, SYS + 0xB44, SYS + 0xB48);
    }
    outputs_apply();
    acquisition_dispatcher();
    any = f1 | f2;
    if (any) {
        if (f1 && !f2)
            SI(trigger_channel) = 0;
        if (!f1 && f2)
            SI(trigger_channel) = 1;
        SI(f_275) = 1;
        SI(f_274) = 1;
        SI(trigger_level) = (unsigned short)W32(meas + 0xC);
        W32(meas + 0x18) = (W32(meas + 8) << 1) / 3u;
        for (; tb <= tb_end; tb++) {
            SI(timebase_index) = tb;
            box(cx, cy);
            if ((int)W32(meas + 0x10) >= 6)
                break;
        }
        trigger_level_auto();
        B8(SAVED + 0x27C) = SI(trigger_channel);
        H16(SAVED + 0x27E) = SI(trigger_level);
        B8(SAVED + 0x588) = SI(timebase_index);
    }
    B8(SAVED + 0x3C) = SI(acq_transition_flag);
    B8(SAVED + 0x665) = SI(f_665);
    B8(SAVED + 0x660) = SI(hires_option);
    B8(SAVED + 0x27A) = 0;
    g_touch->enabled = 1;
    if (any) {
        a = W32(SYS + 0x570);
        b = W32(SYS + 0x574);
        W32(SAVED + 0x570) = a;
        W32(SAVED + 0x574) = b;
    }
    if (SI(ch1_enabled) && f1) {
        B8(SAVED + 0x41) = SI(ch1_vdiv_index);
        W32(SAVED + 0x60) = SI(ch1_offset);
    }
    if (SI(ch2_enabled) && f2) {
        B8(SAVED + 0x105) = SI(ch2_vdiv_index);
        W32(SAVED + 0x120) = SI(ch2_offset);
    }
    state_restore_all();
}
