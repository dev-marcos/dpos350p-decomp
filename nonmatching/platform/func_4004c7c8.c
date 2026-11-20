/*
 * auto_set_from -- stock 0x4004C7C8 (1856 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Body of auto_set, entered with r0 = the settings block 0x40544E68 (only its plot box +4 .. +0xA is read through it; it gives the centre of the message box). state_backup_all, fb_target_back, settings reset for the search (both offsets SYS+0x60 / +0x120 = 250, ranges SYS+0x41 / +0x105 = 0, time base SYS+0x588 = 0x1F, ...). Then for each volts/div range i < SYS+0x1BA (stopping a channel once its trace no longer fits): redraw with the message box, pct = 100 x int64 SYS+0xEF0 + 8i / int64 SYS+0xEF8 + 8i, and percent_at_least on the trace extent above / below the offset (SYS+0xB28 / +0xB2C for CH1, +0xB44 / +0xB48 for CH2) moves the offset. Each enabled channel whose amplitude word (SYS+0xB30 / +0xB4C) is at least 51 is 'found'; its offset moves the trace centre (SYS+0xB34 / +0xB50) to the row of the layout SYS+0x1B0 (1: half the height for both, 2: 3/4 for CH1 and 1/4 for CH2), clamped to 30 .. height - 30. With a channel found the trigger source SYS+0x27C follows it (unchanged when both), the trigger level comes from the measurement block, and the time base steps SYS+0x2A8 .. SYS+0x2A9 - 1 until the period count word (+0x10) is at least 6; trigger_level_auto. The results are copied into the backup block 0x409223F0 before the tail call to state_restore_all. Layouts other than 1 / 2 leave the target row uninitialised in the stock code (OPEN; 0 here).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define TCH 0x42E68BB0u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define S8(a) (*(volatile signed char *)(a))
#define W32(a) (*(volatile u32 *)(a))

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
