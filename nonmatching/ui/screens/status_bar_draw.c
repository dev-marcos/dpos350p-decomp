/*
 * status_bar_draw -- stock 0x4001AD50 (5256 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Window 3 (layout 0x42E64520 + 0x30), the top status bar: seven 130 x 44 tiles, 134 px apart, each with a 40 x 18 coloured name tag and a touch button (0x42E61FF8 + 0xAC + 10 i, filled with theme colour 2 while pressed). CH1 / CH2 (on: SYS+0x3E / +0x102): the bandwidth limit SYS64(0x68 / 0x128) in kHz with BW, the volts/div string SYS+0x70 / +0x130, the coupling DC/AC (SYS+0x3F / +0x103) and the probe 1X/10X/100X (SYS+0x40 / +0x104). HOR: the time base SI(timebase_period) (with an M = prefix unless the zoom flag SYS+0x2AA is set, then also the zoomed time base SYS64(0x5A0) on the right). ACQ: Run / Stop (SYS+0x3C), the sample rate SI(f_538) * SI(sample_divisor) * 100 / 95 S/s and SI(hires_option) + 8 bit. TRI: the source CH1/CH2 (SYS+0x27C), the mode Auto/Single/Normal (SYS+0x27A), a rising or falling edge icon (SYS+0x27B) and the HF reject label HF1..HF3 (SYS+0x284). GEN: the waveform name (table 0x40094EF0, 30 bytes each, index G+0x2EFA), the amplitude GN32(0x2F00) and frequency GN32(0x2EFC). MOV: UI text 65 or 66 by SYS+0x671. Values are formatted with value_si_scale and drawn with the shadowed number/text helpers in font 0x42E658D8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "util.h"
#include "sys_info.h"
#include "state/gen_block.h"
#include "state/ui_windows.h"
#include "state/ui_texts.h"
#include "state/theme.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
u32 number_text_width_mode0_raw(u32 lo, u32 hi, u32 dec) __asm__("number_text_width_mode0");
void str_copy_raw(char *d, const char *s) __asm__("str_copy");

#define SYSB      0x40544E68u
#define U16(v)    ((v) & 0xFFFFu)
#define S8(o)     (*(volatile unsigned char *)(SYSB + (o)))
#define S64(o)    (*(volatile unsigned long long *)(SYSB + (o)))
#define GN32(o)    (*(volatile int *)(0x42E3F984u + (o)))
#define TREC(o)   ((volatile unsigned char *)(0x42E61FF8u + (o)))
#define TXE(k)    (0x42C2FE74u + (k) * 0x1A0u)
#define TXP(k)    (*(const char *volatile *)TXE(k))

struct sbc {
    long long v;
    int dec, pre;
    char buf[20];
};

static void sb_text(u32 x, u32 y, const char *s)
{
    text_shadow_draw(U16(x), U16(y), s, 1, 2, 0);
}

static void sb_num(struct sbc *c, u32 x, u32 y)
{
    number_mode0_shadow_draw(U16(x), U16(y), (u32)c->v, (u32)((unsigned long long)c->v >> 32),
                          (unsigned char)c->dec, 1, 2, 0);
}

static u32 sb_numw(struct sbc *c)
{
    return number_text_width_mode0_raw((u32)c->v, (u32)((unsigned long long)c->v >> 32), (unsigned char)c->dec);
}

/* drop one decimal (the values are scaled by 1000 before value_si_scale) */
static void sb_drop(struct sbc *c)
{
    if (c->dec) {
        c->v = (long long)((unsigned long long)c->v / 10u);
        c->dec--;
    }
}

/* background, the coloured name tag and the black name */
static void sb_tile(u32 x, u32 y, u32 tag, const char *name)
{
    draw_colour_set(THEME_WORD(0x14));
    rect_fill(U16(x), U16(y), 0x82, 0x2C);
    draw_colour_set(tag);
    rect_fill(U16(x), U16(y), 0x28, 0x12);
    draw_colour_set(0);
    text_draw(U16(x + 7), U16(y + 2), name);
}

static void sb_button(u32 x, u32 y, u32 rec)
{
    touch_button_set_rect(U16(x), U16(y), 0x82, 0x2C, TREC(rec));
    if (TREC(rec)[1] == 1) {
        draw_colour_set(THEME_PRESSED);
        rect_fill(U16(x), U16(y), 0x82, 0x2C);
    }
}

static void sb_channel(struct sbc *c, u32 x, u32 y, u32 base, u32 bw, u32 on_colour, const char *name, u32 rec)
{
    u32 tw, r8;

    draw_colour_set(THEME_WORD(0x14));
    rect_fill(U16(x), U16(y), 0x82, 0x2C);
    draw_colour_set(S8(base) ? on_colour : 0x888888);
    rect_fill(U16(x), U16(y), 0x28, 0x12);
    draw_colour_set(0);
    text_draw(U16(x + 7), U16(y + 2), name);
    draw_colour_set(THEME_WORD(0x18));
    if (S8(base)) {                                           /* bandwidth limit */
        value_si_scale(7, 0, (long long)(S64(bw) * 1000u), &c->v, &c->dec, &c->pre);
        if (c->pre == 1)
            str_copy_raw(c->buf, "Hz");
        else if (c->pre == 2)
            str_copy_raw(c->buf, "K");
        else if (c->pre == 3)
            str_copy_raw(c->buf, "M");
        sb_drop(c);
        tw = text_width(c->buf);
        r8 = x + 0x79 - (sb_numw(c) + tw);
        text_draw(U16(r8 - 0x1D), U16(y + 2), "BW");
        sb_num(c, r8, y + 2);
        sb_text(x + 0x7B - text_width(c->buf), y + 2, c->buf);
    }
    draw_colour_set(THEME_WORD(0x18));
    sb_text(x + 7, y + 0x19, (const char *)(SYSB + bw + 8));          /* volts / div */
    str_copy_raw(c->buf, S8(base + 1) == 0 ? "DC" : "AC");
    sb_text(x + 0x45, y + 0x19, c->buf);
    if (S8(base + 2) == 0)
        str_copy_raw(c->buf, "1X");
    else if (S8(base + 2) == 1)
        str_copy_raw(c->buf, "10X");
    else if (S8(base + 2) == 2)
        str_copy_raw(c->buf, "100X");
    sb_text(x + 0x7B - text_width(c->buf), y + 0x19, c->buf);
    sb_button(x, y, rec);
}

static void sb_time_unit(struct sbc *c)
{
    switch (c->pre) {
    case 0: str_copy_raw(c->buf, "pS"); break;
    case 1: str_copy_raw(c->buf, "nS"); break;
    case 2: str_copy_raw(c->buf, "uS"); break;
    case 3: str_copy_raw(c->buf, "mS"); break;
    case 4: str_copy_raw(c->buf, "S"); break;
    }
}

void status_bar_draw(void)
{
    u32 x = WREC(3, x), y = WREC(3, y) + 3, y2, y1;
    u32 w, fx, sl, nx;
    const char *s;
    struct sbc st, *c = &st;

    y1 = y + 0x19;
    font_set(0x42E658D8u);
    text_spacing_set(2);

    sb_channel(c, x, y, 0x3E, 0x68, 0xFFFF00, "CH1", 0xAC);
    x = U16(x + 0x86);
    sb_channel(c, x, y, 0x102, 0x128, 0x00FFFF, "CH2", 0xB6);

    /* horizontal: time base, zoomed time base */
    x = U16(x + 0x86);
    sb_tile(x, y, 0x3388FF, "HOR");
    draw_colour_set(THEME_WORD(0x18));
    value_si_scale(8, 0, (long long)S64(0x590), &c->v, &c->dec, &c->pre);
    sb_time_unit(c);
    sl = sb_numw(c) + x;
    if (S8(0x2AA) == 0) {
        sb_text(x + 7, y1, "M = ");
        fx = text_width("M = ") + x;
        sb_num(c, fx + 7, y1);
        nx = text_width("M = ") + sl + 9;
    } else {
        sb_num(c, x + 7, y1);
        nx = sl + 9;
    }
    sb_text(nx, y1, c->buf);
    if (S8(0x2AA)) {
        value_si_scale(8, 0, (long long)S64(0x5A0), &c->v, &c->dec, &c->pre);
        sb_time_unit(c);
        w = x + 0x79 - text_width(c->buf);
        sb_num(c, w - sb_numw(c), y1);
        sb_text(x + 0x7B - text_width(c->buf), y1, c->buf);
    }
    sb_button(x, y, 0xC0);

    /* acquisition: run state, sample rate, resolution */
    x = U16(x + 0x86);
    sb_tile(x, y, 0xFF8833, "ACQ");
    if (S8(0x3C) == 0) {
        s = "Run";
        draw_colour_set(THEME_WORD(0x18));
    } else {
        draw_colour_set(0xFF0000);
        s = "Stop";
    }
    sb_text(x + 0x7B - text_width(s), y + 2, s);
    draw_colour_set(THEME_WORD(0x18));
    value_si_scale(8, 0, (long long)(S64(0x538) * S8(0x1B0) * 100u / 0x5Fu), &c->v, &c->dec, &c->pre);
    switch (c->pre) {
    case 1: str_copy_raw(c->buf, "S/s"); break;
    case 2: str_copy_raw(c->buf, "KS/s"); break;
    case 3: str_copy_raw(c->buf, "MS/s"); break;
    case 4: str_copy_raw(c->buf, "GS/s"); break;
    }
    w = sb_numw(c);
    sb_num(c, x + 7, y1);
    sb_text(w + x + 0xA, y1, c->buf);
    fx = text_width("bit");
    w = number_text_width_mode0_raw(S8(0x660) + 8, 0, 0);
    number_mode0_shadow_draw(U16(x + 0x77 - fx - w), U16(y1), S8(0x660) + 8, 0, 0, 1, 2, 0);
    sb_text(x + 0x7B - text_width("bit"), y1, "bit");
    sb_button(x, y, 0xCA);

    /* trigger: source, mode, edge, HF reject */
    x = U16(x + 0x86);
    sb_tile(x, y, 0xAA44FF, "TRI");
    if (S8(0x27C) == 0) {
        draw_colour_set(0xFFFF00);
        s = "CH1";
    } else {
        draw_colour_set(0x00FFFF);
        s = "CH2";
    }
    sb_text(x + 0x7B - text_width(s), y + 2, s);
    draw_colour_set(THEME_WORD(0x18));
    if (S8(0x27A) == 0)
        str_copy_raw(c->buf, "Auto");
    else if (S8(0x27A) == 1)
        str_copy_raw(c->buf, "Single");
    else if (S8(0x27A) == 2)
        str_copy_raw(c->buf, "Normal");
    sb_text(x + 7, y1, c->buf);
    fx = x + 0x49;
    y2 = y + 0x25;
    if (S8(0x27B) == 0) {                                     /* rising edge */
        vline_draw(U16(fx), U16(y + 0x1A), U16(y2));
        hspan_fill(U16(y + 0x1A), U16(fx), U16(x + 0x4D));
        hspan_fill(U16(y2), U16(x + 0x45), U16(fx));
        hspan_fill(U16(y + 0x1E), U16(x + 0x48), U16(x + 0x4A));
        hspan_fill(U16(y + 0x1F), U16(x + 0x47), U16(x + 0x4B));
        hspan_fill(U16(y + 0x20), U16(x + 0x46), U16(x + 0x4C));
    } else {                                                  /* falling edge */
        vline_draw(U16(fx), U16(y + 0x1A), U16(y2));
        hspan_fill(U16(y + 0x1A), U16(x + 0x45), U16(fx));
        hspan_fill(U16(y2), U16(fx), U16(x + 0x4D));
        hspan_fill(U16(y + 0x1F), U16(x + 0x46), U16(x + 0x4C));
        hspan_fill(U16(y + 0x20), U16(x + 0x47), U16(x + 0x4B));
        hspan_fill(U16(y + 0x21), U16(x + 0x48), U16(x + 0x4A));
    }
    s = 0;
    if (S8(0x284) == 1)
        s = "HF1";
    else if (S8(0x284) == 2)
        s = "HF2";
    else if (S8(0x284) == 3)
        s = "HF3";
    if (s)
        sb_text(x + 0x7B - text_width(s), y1, s);
    sb_button(x, y, 0xD4);

    /* generator: waveform, amplitude, frequency */
    x = x + 0x86;
    sb_tile(x, y, 0xFF3388, "GEN");
    draw_colour_set(0x5555FF);
    text_spacing_set(1);
    w = text_width((const char *)(IMGU(40094ef0) + GENS(waveform) * 30u));
    sb_text(x + 0x7B - w, y + 2, (const char *)(IMGU(40094ef0) + GENS(waveform) * 30u));
    draw_colour_set(THEME_WORD(0x18));
    text_spacing_set(2);
    value_si_scale(6, 0, (long long)GN32(0x2F00) * 1000, &c->v, &c->dec, &c->pre);
    switch (c->pre) {
    case 0: str_copy_raw(c->buf, "uV"); break;
    case 1: str_copy_raw(c->buf, "mV"); break;
    case 2: str_copy_raw(c->buf, "V"); break;
    case 3: str_copy_raw(c->buf, "KV"); break;
    }
    w = sb_numw(c);
    sb_num(c, x + 7, y1);
    sb_text(w + x + 0xA, y1, c->buf);
    value_si_scale(7, 0, (long long)GN32(0x2EFC) * 1000, &c->v, &c->dec, &c->pre);
    switch (c->pre) {
    case 0:
    case 1: str_copy_raw(c->buf, "Hz"); break;
    case 2: str_copy_raw(c->buf, "KHz"); break;
    case 3: str_copy_raw(c->buf, "MHz"); break;
    case 4: str_copy_raw(c->buf, "GHz"); break;
    }
    sb_drop(c);
    w = sb_numw(c);
    sb_num(c, x + 0x43, y1);
    sb_text(w + x + 0x46, y1, c->buf);
    sb_button(x, y, 0xDE);

    /* movement / persistence label */
    x = x + 0x86;
    sb_tile(x, y, 0x33FF88, "MOV");
    draw_colour_set(THEME_WORD(0x18));
    if (S8(0x671) == 0)
        sb_text(x + TEXT_DX(UI_TEXT(65)) + 7, y + TEXT_DY(UI_TEXT(65)) + 0x19, TXP(65));
    else
        sb_text(x + TEXT_DX(UI_TEXT(66)) + 7, y + TEXT_DY(UI_TEXT(66)) + 0x19, TXP(66));
    sb_button(x, y, 0xE8);
}
