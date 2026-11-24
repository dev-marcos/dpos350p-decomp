/*
 * cursor_panel_draw -- stock 0x40031794 (4800 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Cursor read-out panel, drawn over the trace area unless touch byte 0x42E68BB0 + 0x17 or SYS+0x652 is set, or the scope is in the one-shot mode SYS+0x28 while running (SYS+0x3C clear), or neither the time cursors (SYS+0x802) nor the voltage cursors (SYS+0x803) are on. First it converts the cursor positions: voltage V = (screen bottom SI(plot_height) - 2 - channel offset SI(ch1_offset) / SI(ch2_offset) - cursor y SI(cursor_y1) / SI(cursor_y2)) * volts-per-division SI(ch1_scale) / SI(ch2_scale) / pixels-per-division SI(grid_step) into SYS64(0x838 / 0x840) for CH1 and SYS64(0x848 / 0x850) for CH2, with the differences in SYS64(0x858 / 0x860); time T = (cursor x SYS32(0x808 / 0x80C) - left edge SI(plot_x_mid)) * time base SI(timebase_period) / SI(f_01c) into SYS64(0x818 / 0x820), dT in SI(f_830) and 1 / dT (1e15 / dT) in SI(f_828). Then it draws, at SI(plot_x0) + 1 / SI(plot_y0) + 1 (or + 0xA9 with zoom, rows 25 instead of 33 px), 145 px wide alternating rows: HOR H1 H2 D-T D-F, then per enabled channel CH1/CH2 V1 V2 D-V, labels in grey 0x999999 (font 0x42E65418) and values in white (font 0x42E66258), differences as absolute values.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "sys_info.h"

#define SYSB      0x40544E68u
#define U16(v)    ((v) & 0xFFFFu)
#define S8(o)     (*(volatile unsigned char *)(SYSB + (o)))
#define S16(o)    (*(volatile unsigned short *)(SYSB + (o)))
#define S32(o)    (*(volatile int *)(SYSB + (o)))
#define S64(o)    (*(volatile long long *)(SYSB + (o)))
#define TEXT_W    (*(volatile unsigned short *)0x42E68BE4u)

#define CP_GREY   0x999999u
#define CP_LFONT  0x42E65418u
#define CP_VFONT  0x42E66258u

struct cpc {
    long long v;
    int dec, pre;
};

/* unit kinds: 0 time, 1 frequency, 2 voltage */
static const char *cp_unit(u32 kind, int pre)
{
    if (kind == 0) {
        switch (pre) {
        case 0: return "pS";
        case 1: return "nS";
        case 2: return "uS";
        case 3: return "mS";
        case 4: return "S";
        }
    } else if (kind == 1) {
        switch (pre) {
        case 0: return "mHz";
        case 1: return "Hz";
        case 2: return "KHz";
        case 3: return "MHz";
        case 4: return "GHz";
        }
    } else {
        switch (pre) {
        case 0: return "uV";
        case 1: return "mV";
        case 2: return "V";
        case 4: return "kV";
        }
    }
    return 0;
}

static void cp_fill(u32 px, u32 y, u32 rh)
{
    colour_toggle_42e68d1a();
    rect_fill(U16(px), U16(y), 0x91, U16(rh));
}

/* a block of n rows starting at y, the first one with a header label */
static void cp_block(u32 px, u32 y, u32 rh, u32 n, const char *name)
{
    u32 i;

    cp_fill(px, y, rh);
    draw_colour_set(0x555555);
    text_draw(U16(px + 8), U16(y + (int)(rh - 12) / 2), name);
    for (i = 1; i < n; i++)
        cp_fill(px, y + i * rh, rh);
}

#define CP_ABS   1   /* draw |v| with the flag-0 number helper */
#define CP_ZF    2   /* a zero difference also clears 1/dT */

static void cp_row(struct cpc *c, u32 kind, u32 off, u32 x8, u32 top, u32 rh, const char *l1, const char *l2,
                   u32 l2dx, u32 numx, u32 flags, u32 unit)
{
    u32 ly = top + (int)(rh - 12) / 2, ny = top + (int)(rh - 17) / 2;
    const char *s;

    value_si_scale(kind, 0, S64(off), &c->v, &c->dec, &c->pre);
    draw_colour_set(CP_GREY);
    word_42e68c74_set(CP_LFONT);
    text_draw(U16(x8), U16(ly), l1);
    if (l2)
        text_draw(U16(TEXT_W + x8 + l2dx), U16(ly), l2);
    draw_colour_set(0xFFFFFF);
    word_42e68c74_set(CP_VFONT);
    half_42e68be2_set(3);
    if (flags & CP_ABS) {
        if (c->v < 0)
            c->v = -c->v;
        if ((flags & CP_ZF) && c->v == 0) {
            S32(0x828) = 0;
            S32(0x82C) = 0;
        }
        number_draw_mode0(U16(numx), U16(ny), (u32)c->v, (u32)((unsigned long long)c->v >> 32),
                            (unsigned char)c->dec);
    } else {
        number_draw_mode1(U16(numx), U16(ny), (u32)c->v, (u32)((unsigned long long)c->v >> 32),
                            (unsigned char)c->dec);
    }
    half_42e68be2_set(2);
    numx = numx + TEXT_W + 7;
    draw_colour_set(CP_GREY);
    word_42e68c74_set(CP_LFONT);
    s = cp_unit(unit, c->pre);
    if (s)
        text_draw(U16(numx), U16(ly), s);
}

/* V1, V2 and D-V of one channel below its header row at y */
static void cp_volts(struct cpc *c, u32 x8, u32 y, u32 rh, u32 off, u32 dv)
{
    cp_row(c, 6, off, x8, y + rh, rh, "V", "1", 1, x8 + 0x1C, 0, 2);
    cp_row(c, 6, off + 8, x8, y + 2 * rh, rh, "V", "2", 0, x8 + 0x1C, 0, 2);
    cp_row(c, 6, dv, x8, y + 3 * rh, rh, "D-V", 0, 0, x8 + 0x23, CP_ABS, 2);
}

void cursor_panel_draw(void)
{
    u32 px, py, rh, y, d, ts;
    long long a, t, b, m, q1, q2, q3, q4, tm, t1, t2, dt;
    struct cpc st, *c = &st;

    if (g_touch->serviced)
        return;
    if (S8(0x652))
        return;
    if (S8(0x28) && S8(0x3C) == 0)
        return;
    if (S8(0x802) == 0 && S8(0x803) == 0)
        return;
    *(volatile unsigned char *)0x42E68D1Au = 0;
    half_42e68be2_set(2);
    px = S16(4) + 1;
    if (S8(0x2AA) == 0) {
        py = S16(6) + 1;
        rh = 0x21;
    } else {
        py = S16(6) + 0xA9;
        rh = 0x19;
    }

    /* cursor positions to volts and seconds */
    a = (long long)S16(0xA) - 2;
    t = a - S32(0x60);
    d = S16(0x20);
    m = S64(0x50);
    q1 = (t - S32(0x810)) * m / (long long)d;
    S64(0x838) = q1;
    q2 = (t - S32(0x814)) * m / (long long)d;
    S64(0x840) = q2;
    b = a - S32(0x120);
    q3 = (b - S32(0x810)) * S64(0x110) / (long long)d;
    S64(0x848) = q3;
    q4 = (b - S32(0x814)) * S64(0x110) / (long long)d;
    S64(0x850) = q4;
    S64(0x858) = q2 - q1;
    S64(0x860) = q4 - q3;
    tm = S64(0x590);
    ts = S16(0x1C);
    t1 = (long long)(int)(S32(0x808) - S16(0xC)) * tm / (long long)ts;
    S64(0x818) = t1;
    t2 = (long long)(int)(S32(0x80C) - S16(0xC)) * tm / (long long)ts;
    S32(0x820) = (int)t2;
    dt = t2 - t1;
    S64(0x828) = 1000000000000000LL / dt;
    S64(0x830) = dt;
    S32(0x824) = (int)((unsigned long long)t2 >> 32);

    /* row backgrounds and headers */
    half_42e68be2_set(2);
    word_42e68c74_set(CP_LFONT);
    y = py;
    if (S8(0x802)) {
        cp_block(px, y, rh, 5, "HOR");
        y += 5 * rh;
    }
    if (S8(0x803)) {
        if (S8(0x3E)) {
            cp_block(px, y, rh, 4, "CH1");
            y += 4 * rh;
        }
        if (S8(0x102))
            cp_block(px, y, rh, 4, "CH2");
    }

    /* values */
    half_42e68be2_set(2);
    y = py;
    if (S8(0x802)) {
        cp_row(c, 8, 0x818, px + 8, y + rh, rh, "H", "1", 1, px + 0x24, 0, 0);
        cp_row(c, 8, 0x820, px + 8, y + 2 * rh, rh, "H2", 0, 0, px + 0x24, 0, 0);
        cp_row(c, 8, 0x830, px + 8, y + 3 * rh, rh, "D-T", 0, 0, px + 0x2B, CP_ABS | CP_ZF, 0);
        cp_row(c, 7, 0x828, px + 8, y + 4 * rh, rh, "D-F", 0, 0, px + 0x2B, CP_ABS, 1);
        y += 5 * rh;
    }
    if (S8(0x803) == 0)
        return;
    if (S8(0x3E)) {
        cp_volts(c, px + 8, y, rh, 0x838, 0x858);
        y += 4 * rh;
    }
    if (S8(0x102))
        cp_volts(c, px + 8, y, rh, 0x848, 0x860);
}
