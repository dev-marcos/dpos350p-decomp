# Window 3: the top status bar -- seven 130 x 44 tiles CH1, CH2, HOR, ACQ, TRI, GEN, MOV.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)

f(0x4001AD50, 5256, "status_bar_draw", "ui",
  "Window 3 (layout 0x42E64520 + 0x30), the top status bar: seven 130 x 44 tiles, 134 px apart, each with a "
  "40 x 18 coloured name tag and a touch button (0x42E61FF8 + 0xAC + 10 i, filled with theme colour 2 while "
  "pressed). CH1 / CH2 (on: SYS+0x3E / +0x102): the bandwidth limit SYS64(0x68 / 0x128) in kHz with BW, the "
  "volts/div string SYS+0x70 / +0x130, the coupling DC/AC (SYS+0x3F / +0x103) and the probe 1X/10X/100X "
  "(SYS+0x40 / +0x104). HOR: the time base SYS64(0x590) (with an M = prefix unless the zoom flag SYS+0x2AA is "
  "set, then also the zoomed time base SYS64(0x5A0) on the right). ACQ: Run / Stop (SYS+0x3C), the sample rate "
  "SYS64(0x538) * SYS8(0x1B0) * 100 / 95 S/s and SYS8(0x660) + 8 bit. TRI: the source CH1/CH2 (SYS+0x27C), "
  "the mode Auto/Single/Normal (SYS+0x27A), a rising or falling edge icon (SYS+0x27B) and the HF reject "
  "label HF1..HF3 (SYS+0x284). GEN: the waveform name (table 0x40094EF0, 30 bytes each, index G+0x2EFA), "
  "the amplitude GN32(0x2F00) and frequency GN32(0x2EFC). MOV: UI text 65 or 66 by SYS+0x671. Values are "
  "formatted with value_si_scale and drawn with the shadowed number/text helpers in font 0x42E658D8.",
  r'''#include "hw/fpga.h"

void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void vline_draw(u32 x, u32 y0, u32 y1);
void func_4002ecc4_span(u32 y, u32 x0, u32 x1);
void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void text_draw(u32 x, u32 y, const char *s);
u32 text_width(const char *s);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void text_d6f8_shadow_draw(u32 x, u32 y, u32 a, u32 b, unsigned char e, unsigned char n, unsigned char m,
                           unsigned char alpha);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void value_si_scale(u32 kind, u32 unused, long long v, long long *out, int *dec, int *prefix);
u32 func_4002d2d4_d0(u32 lo, u32 hi, u32 dec);
void str_copy(char *d, const char *s);

#define SYSB      0x40544E68u
#define U16(v)    ((v) & 0xFFFFu)
#define LAY16(o)  (*(volatile unsigned short *)(0x42E64520u + (o)))
#define THEME(i)  (*(volatile u32 *)(SYSB + 0x3DD544u + 4 * (i)))
#define S8(o)     (*(volatile unsigned char *)(SYSB + (o)))
#define S64(o)    (*(volatile unsigned long long *)(SYSB + (o)))
#define GN8(o)     (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GN32(o)    (*(volatile int *)(0x42E3F984u + (o)))
#define TREC(o)   ((volatile unsigned char *)(0x42E61FF8u + (o)))
#define TXE(k)    (0x42C2FE74u + (k) * 0x1A0u)
#define TXP(k)    (*(const char *volatile *)TXE(k))
#define TDX(k)    (*(volatile signed char *)(TXE(k) + 0x194))
#define TDY(k)    (*(volatile signed char *)(TXE(k) + 0x195))

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
    text_d6f8_shadow_draw(U16(x), U16(y), (u32)c->v, (u32)((unsigned long long)c->v >> 32),
                          (unsigned char)c->dec, 1, 2, 0);
}

static u32 sb_numw(struct sbc *c)
{
    return func_4002d2d4_d0((u32)c->v, (u32)((unsigned long long)c->v >> 32), (unsigned char)c->dec);
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
    draw_colour_set(THEME(0));
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
        draw_colour_set(THEME(2));
        rect_fill(U16(x), U16(y), 0x82, 0x2C);
    }
}

static void sb_channel(struct sbc *c, u32 x, u32 y, u32 base, u32 bw, u32 on_colour, const char *name, u32 rec)
{
    u32 tw, r8;

    draw_colour_set(THEME(0));
    rect_fill(U16(x), U16(y), 0x82, 0x2C);
    draw_colour_set(S8(base) ? on_colour : 0x888888);
    rect_fill(U16(x), U16(y), 0x28, 0x12);
    draw_colour_set(0);
    text_draw(U16(x + 7), U16(y + 2), name);
    draw_colour_set(THEME(1));
    if (S8(base)) {                                           /* bandwidth limit */
        value_si_scale(7, 0, (long long)(S64(bw) * 1000u), &c->v, &c->dec, &c->pre);
        if (c->pre == 1)
            str_copy(c->buf, "Hz");
        else if (c->pre == 2)
            str_copy(c->buf, "K");
        else if (c->pre == 3)
            str_copy(c->buf, "M");
        sb_drop(c);
        tw = text_width(c->buf);
        r8 = x + 0x79 - (sb_numw(c) + tw);
        text_draw(U16(r8 - 0x1D), U16(y + 2), "BW");
        sb_num(c, r8, y + 2);
        sb_text(x + 0x7B - text_width(c->buf), y + 2, c->buf);
    }
    draw_colour_set(THEME(1));
    sb_text(x + 7, y + 0x19, (const char *)(SYSB + bw + 8));          /* volts / div */
    str_copy(c->buf, S8(base + 1) == 0 ? "DC" : "AC");
    sb_text(x + 0x45, y + 0x19, c->buf);
    if (S8(base + 2) == 0)
        str_copy(c->buf, "1X");
    else if (S8(base + 2) == 1)
        str_copy(c->buf, "10X");
    else if (S8(base + 2) == 2)
        str_copy(c->buf, "100X");
    sb_text(x + 0x7B - text_width(c->buf), y + 0x19, c->buf);
    sb_button(x, y, rec);
}

static void sb_time_unit(struct sbc *c)
{
    switch (c->pre) {
    case 0: str_copy(c->buf, "pS"); break;
    case 1: str_copy(c->buf, "nS"); break;
    case 2: str_copy(c->buf, "uS"); break;
    case 3: str_copy(c->buf, "mS"); break;
    case 4: str_copy(c->buf, "S"); break;
    }
}

void status_bar_draw(void)
{
    u32 x = LAY16(0x30), y = LAY16(0x32) + 3, y2, y1;
    u32 w, fx, sl, nx;
    const char *s;
    struct sbc st, *c = &st;

    y1 = y + 0x19;
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);

    sb_channel(c, x, y, 0x3E, 0x68, 0xFFFF00, "CH1", 0xAC);
    x = U16(x + 0x86);
    sb_channel(c, x, y, 0x102, 0x128, 0x00FFFF, "CH2", 0xB6);

    /* horizontal: time base, zoomed time base */
    x = U16(x + 0x86);
    sb_tile(x, y, 0x3388FF, "HOR");
    draw_colour_set(THEME(1));
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
        draw_colour_set(THEME(1));
    } else {
        draw_colour_set(0xFF0000);
        s = "Stop";
    }
    sb_text(x + 0x7B - text_width(s), y + 2, s);
    draw_colour_set(THEME(1));
    value_si_scale(8, 0, (long long)(S64(0x538) * S8(0x1B0) * 100u / 0x5Fu), &c->v, &c->dec, &c->pre);
    switch (c->pre) {
    case 1: str_copy(c->buf, "S/s"); break;
    case 2: str_copy(c->buf, "KS/s"); break;
    case 3: str_copy(c->buf, "MS/s"); break;
    case 4: str_copy(c->buf, "GS/s"); break;
    }
    w = sb_numw(c);
    sb_num(c, x + 7, y1);
    sb_text(w + x + 0xA, y1, c->buf);
    fx = text_width("bit");
    w = func_4002d2d4_d0(S8(0x660) + 8, 0, 0);
    text_d6f8_shadow_draw(U16(x + 0x77 - fx - w), U16(y1), S8(0x660) + 8, 0, 0, 1, 2, 0);
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
    draw_colour_set(THEME(1));
    if (S8(0x27A) == 0)
        str_copy(c->buf, "Auto");
    else if (S8(0x27A) == 1)
        str_copy(c->buf, "Single");
    else if (S8(0x27A) == 2)
        str_copy(c->buf, "Normal");
    sb_text(x + 7, y1, c->buf);
    fx = x + 0x49;
    y2 = y + 0x25;
    if (S8(0x27B) == 0) {                                     /* rising edge */
        vline_draw(U16(fx), U16(y + 0x1A), U16(y2));
        func_4002ecc4_span(U16(y + 0x1A), U16(fx), U16(x + 0x4D));
        func_4002ecc4_span(U16(y2), U16(x + 0x45), U16(fx));
        func_4002ecc4_span(U16(y + 0x1E), U16(x + 0x48), U16(x + 0x4A));
        func_4002ecc4_span(U16(y + 0x1F), U16(x + 0x47), U16(x + 0x4B));
        func_4002ecc4_span(U16(y + 0x20), U16(x + 0x46), U16(x + 0x4C));
    } else {                                                  /* falling edge */
        vline_draw(U16(fx), U16(y + 0x1A), U16(y2));
        func_4002ecc4_span(U16(y + 0x1A), U16(x + 0x45), U16(fx));
        func_4002ecc4_span(U16(y2), U16(fx), U16(x + 0x4D));
        func_4002ecc4_span(U16(y + 0x1F), U16(x + 0x46), U16(x + 0x4C));
        func_4002ecc4_span(U16(y + 0x20), U16(x + 0x47), U16(x + 0x4B));
        func_4002ecc4_span(U16(y + 0x21), U16(x + 0x48), U16(x + 0x4A));
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
    half_42e68be2_set(1);
    w = text_width((const char *)(0x40094EF0u + GN8(0x2EFA) * 30u));
    sb_text(x + 0x7B - w, y + 2, (const char *)(0x40094EF0u + GN8(0x2EFA) * 30u));
    draw_colour_set(THEME(1));
    half_42e68be2_set(2);
    value_si_scale(6, 0, (long long)GN32(0x2F00) * 1000, &c->v, &c->dec, &c->pre);
    switch (c->pre) {
    case 0: str_copy(c->buf, "uV"); break;
    case 1: str_copy(c->buf, "mV"); break;
    case 2: str_copy(c->buf, "V"); break;
    case 3: str_copy(c->buf, "KV"); break;
    }
    w = sb_numw(c);
    sb_num(c, x + 7, y1);
    sb_text(w + x + 0xA, y1, c->buf);
    value_si_scale(7, 0, (long long)GN32(0x2EFC) * 1000, &c->v, &c->dec, &c->pre);
    switch (c->pre) {
    case 0:
    case 1: str_copy(c->buf, "Hz"); break;
    case 2: str_copy(c->buf, "KHz"); break;
    case 3: str_copy(c->buf, "MHz"); break;
    case 4: str_copy(c->buf, "GHz"); break;
    }
    sb_drop(c);
    w = sb_numw(c);
    sb_num(c, x + 0x43, y1);
    sb_text(w + x + 0x46, y1, c->buf);
    sb_button(x, y, 0xDE);

    /* movement / persistence label */
    x = x + 0x86;
    sb_tile(x, y, 0x33FF88, "MOV");
    draw_colour_set(THEME(1));
    if (S8(0x671) == 0)
        sb_text(x + TDX(65) + 7, y + TDY(65) + 0x19, TXP(65));
    else
        sb_text(x + TDX(66) + 7, y + TDY(66) + 0x19, TXP(66));
    sb_button(x, y, 0xE8);
}
''')
