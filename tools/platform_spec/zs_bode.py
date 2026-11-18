# Value + unit text drawers (phase in degrees, voltage in uV / mV / V, level with 0..2 decimals,
# frequency in Hz .. GHz) and the two Bode plot screens that use them: the grid with its axis
# labels and the cursor panel (C1, C2 and their difference).
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wb(addr, size, name, desc, body, src="display"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


NUM = """
void func_4002d82c_flag1(u32 x, u32 y, u32 lo, u32 hi, u32 dec);
void func_4002d82c_flag0(u32 x, u32 y, u32 lo, u32 hi, u32 dec);
void text_draw(u32 x, u32 y, const char *s);
#define TW (*(volatile unsigned short *)0x42E68BE4u)

static void num(u32 x, u32 y, u32 sg, long long v, u32 dec)
{
    if (sg)
        func_4002d82c_flag1(x, y, (u32)v, (u32)((unsigned long long)v >> 32), dec);
    else
        func_4002d82c_flag0(x, y, (u32)v, (u32)((unsigned long long)v >> 32), dec);
}
"""

wb(0x40038788, 464, "phase_deg_draw",
   "Draws a phase: v = (long long)(d * 1000) clamped to -180000..180000, drawn as v / 10 with 2 decimals when |v| <= 9999, v / 100 with 1 decimal up to 99999, v / 1000 with none up to 999999 "
   "(number_draw with flag 1 when the low byte of sg is set, else flag 0), then the text \"deg\" at x + the last drawn width (u16 0x42E68BE4) + 4.", NUM + """
void phase_deg_draw(u32 x, u32 y, u32 sg, double d)
{
    long long v = (long long)(d * 1000.0);
    sg = (unsigned char)sg;
    if (v < -180000)
        v = -180000;
    else if (v > 180000)
        v = 180000;
    if ((unsigned long long)(v + 9999) < 19999)
        num(x, y, sg, v / 10, 2);
    else if ((unsigned long long)(v + 99999) < 199999)
        num(x, y, sg, v / 100, 1);
    else if ((unsigned long long)(v + 999999) < 1999999)
        num(x, y, sg, v / 1000, 0);
    text_draw((unsigned short)(x + TW + 4), y, "deg");
}
""")

wb(0x40038974, 636, "voltage_uv_draw",
   "Draws a voltage given in microvolts: v = (long long)d; |v| <= 999 as v \"uV\", up to 999999 as v / 1000 \"mV\" (no decimals), up to 9999999 as v / 10000 with 2 decimals \"V\", up to 99999999 as v / 100000 with 1 decimal \"V\"; "
   "larger values draw nothing. Sign flag and unit position as in phase_deg_draw.", NUM + """
void voltage_uv_draw(u32 x, u32 y, u32 sg, double d)
{
    long long v = (long long)d;
    const char *u;
    sg = (unsigned char)sg;
    if ((unsigned long long)(v + 999) < 1999) {
        num(x, y, sg, v, 0);
        u = "uV";
    } else if ((unsigned long long)(v + 999999) < 1999999) {
        num(x, y, sg, v / 1000, 0);
        u = "mV";
    } else if ((unsigned long long)(v + 9999999) < 19999999) {
        num(x, y, sg, v / 10000, 2);
        u = "V";
    } else if ((unsigned long long)(v + 99999999) < 199999999) {
        num(x, y, sg, v / 100000, 1);
        u = "V";
    } else
        return;
    text_draw((unsigned short)(x + TW + 4), y, u);
}
""")

wb(0x40038C00, 428, "level_value_draw",
   "Draws a level without unit: v = (long long)(d * 1000) clamped to -999000..999000, as v / 10 with 2 decimals when |v| <= 9999, v / 100 with 1 decimal up to 99999, else v / 1000 (sign flag as in phase_deg_draw). "
   "Used by level_label_db and level_label_dbmv, which add the unit.", NUM + """
void level_value_draw(u32 x, u32 y, u32 sg, double d)
{
    long long v = (long long)(d * 1000.0);
    sg = (unsigned char)sg;
    if (v < -999000)
        v = -999000;
    else if (v > 999000)
        v = 999000;
    if ((unsigned long long)(v + 9999) < 19999)
        num(x, y, sg, v / 10, 2);
    else if ((unsigned long long)(v + 99999) < 199999)
        num(x, y, sg, v / 100, 1);
    else if ((unsigned long long)(v + 999999) < 1999999)
        num(x, y, sg, v / 1000, 0);
}
""")

wb(0x40038E40, 1016, "frequency_draw",
   "Frequency v = (long long)d in Hz: below 1000 as v \"Hz\"; up to 10^4 / 10^5 / 10^6 as v / 10, 100, 1000 \"KHz\" with 2 / 1 / 0 decimals; then likewise \"MHz\" up to 10^9 and v / 10^7 \"GHz\" with 2 decimals below 10^10; larger values draw nothing. "
   "Except below 1000 the quotient q is bumped by one when q % 10 >= 5 (stock rounding, it looks at the kept digit). With measure = 0 it draws the number (flag 0) and the unit at x + the drawn width + 4 and returns 0; "
   "with measure set it draws nothing and returns number width + unit width + 4.", """
void func_4002d82c_flag0(u32 x, u32 y, u32 lo, u32 hi, u32 dec);
u32 func_4002d2d4_d0(u32 lo, u32 hi, u32 dec);
u32 text_width(const char *s);
void text_draw(u32 x, u32 y, const char *s);
#define TW (*(volatile unsigned short *)0x42E68BE4u)

u32 frequency_draw(u32 x, u32 y, u32 measure, double d)
{
    long long v = (long long)d, q;
    const char *u;
    u32 dec;
    measure = (unsigned char)measure;
    if (v < 1000) {
        q = v;
        dec = 0;
        u = "Hz";
    } else {
        if (v < 10000) {
            q = v / 10;
            dec = 2;
            u = "KHz";
        } else if (v < 100000) {
            q = v / 100;
            dec = 1;
            u = "KHz";
        } else if (v < 1000000) {
            q = v / 1000;
            dec = 0;
            u = "KHz";
        } else if (v < 10000000) {
            q = v / 10000;
            dec = 2;
            u = "MHz";
        } else if (v < 100000000) {
            q = v / 100000;
            dec = 1;
            u = "MHz";
        } else if (v < 1000000000) {
            q = v / 1000000;
            dec = 0;
            u = "MHz";
        } else if (v < 10000000000LL) {
            q = v / 10000000;
            dec = 2;
            u = "GHz";
        } else
            return 0;
        if (q % 10 >= 5)
            q++;
    }
    if (measure)
        return func_4002d2d4_d0((u32)q, (u32)((unsigned long long)q >> 32), dec) + text_width(u) + 4;
    func_4002d82c_flag0(x, y, (u32)q, (u32)((unsigned long long)q >> 32), dec);
    text_draw((unsigned short)(x + TW + 4), y, u);
    return 0;
}
""")

BODE = """
void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void vline_draw(u32 x, u32 y0, u32 y1);
void text_draw(u32 x, u32 y, const char *s);
void level_label_db(u32 x, u32 y, u32 c, double v);
void phase_deg_draw(u32 x, u32 y, u32 sg, double d);
u32 frequency_draw(u32 x, u32 y, u32 measure, double d);
#define BB 0x42E180C8u
#define BH(o) (*(volatile unsigned short *)(BB + (o)))
#define BW(o) (*(volatile int *)(BB + (o)))
#define BD(o) (*(volatile double *)(BB + (o)))
#define U16(v) ((unsigned short)(v))
#define TW (*(volatile unsigned short *)0x42E68BE4u)
"""

wb(0x40038374, 1032, "bode_grid_draw",
   "Unless touch byte 0x42E68BB0+0x17 is set, draws the Bode plot frame from the block 0x42E180C8 (u16 +0 x, +2 y, +4 width, +6 height, +8 decade width, +0xA decades, +0xC row height, +0xE rows; doubles +0x10/+0x18 gain top/bottom, +0x20/+0x28 phase top/bottom, +0x8D08/+0x8D10 log10 of the start / end frequency): "
   "clears the area, draws a double 0x444444 outline, sets touch button 0x42E62B3A to the plot, 3-pixel lines at each decade, the 2..9 minor lines at x + decade * log10(k), the row lines, "
   "the gain labels on the left (level_label_db at x = 2) and the phase labels on the right (phase_deg_draw at x = 943), first and last moved by 7 pixels, and 4 frequency labels 10^(start + i * span / decades) under the plot "
   "(the first left aligned, the last right aligned, the others centred on their decade).", BODE + """
void rect_outline_draw(u32 x, u32 y, u32 w, u32 h);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void func_4002ecc4_span(u32 y, u32 x0, u32 x1);
double log10(double x);
double pow(double x, double y);

void bode_grid_draw(void)
{
    u32 i, j, x, y, w;
    double step, d, base;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0);
    rect_fill(0, U16(BH(2) - 4), 0x3FC, U16(BH(6) + 0x1C));
    draw_colour_set(0x444444);
    rect_outline_draw(BH(0), BH(2), BH(4), BH(6));
    rect_outline_draw(U16(BH(0) + 1), U16(BH(2) + 1), U16(BH(4) + 0xFFFE), U16(BH(6) + 0xFFFE));
    touch_button_set_rect(BH(0), BH(2), BH(4), BH(6), (volatile unsigned char *)0x42E62B3Au);
    x = BH(0);
    y = BH(2) + 1;
    for (i = 0; i < BH(0xA); i++) {
        rect_fill(U16(x), U16(y), 3, U16(BH(6) + 0xFFFE));
        x += BH(8);
    }
    x = BH(0);
    y = BH(2) + 1;
    for (i = 0; i < BH(0xA); i++) {
        base = (double)x;
        for (j = 2; j < 10; j++) {
            d = log10((double)j);
            vline_draw(U16((int)(base + (double)BH(8) * d)), U16(y), U16(BH(6) + y + 0xFFFE));
        }
        x += BH(8);
    }
    x = BH(0) + 1;
    y = BH(2);
    for (i = 1; i < BH(0xE); i++) {
        y += BH(0xC);
        func_4002ecc4_span(U16(y), U16(x), U16(BH(4) + x + 0xFFFD));
    }
    draw_colour_set(0x888888);
    step = (BD(0x10) - BD(0x18)) / (double)BH(0xE);
    y = BH(2) - 7;
    for (i = 0; i <= BH(0xE); i++) {
        d = BD(0x10) - (double)i * step;
        level_label_db(2, i == 0 ? U16(y + 7) : i == BH(0xE) ? U16(y + 0xFFF9) : U16(y), 1, d);
        y += BH(0xC);
    }
    draw_colour_set(0x888888);
    step = (BD(0x20) - BD(0x28)) / (double)BH(0xE);
    y = BH(2) - 7;
    for (i = 0; i <= BH(0xE); i++) {
        d = BD(0x20) - (double)i * step;
        phase_deg_draw(0x3AF, i == 0 ? U16(y + 7) : i == BH(0xE) ? U16(y + 0xFFF9) : U16(y), 1, d);
        y += BH(0xC);
    }
    draw_colour_set(0x888888);
    step = (BD(0x8D10) - BD(0x8D08)) / (double)BH(0xA);
    y = BH(6) + BH(2) + 5;
    x = 0x3AF;
    for (i = 0; i < 4; i++) {
        d = pow(10.0, BD(0x8D08) + (double)i * step);
        w = frequency_draw(U16(x), U16(y), 1, d);
        x = BH(0);
        if (i == 3)
            x = 3 * BH(8) + x - w - 2;
        else if (i != 0)
            x = i * BH(8) + x - (w >> 1);
        frequency_draw(U16(x), U16(y), 0, d);
    }
}
""", src="ui")

wb(0x40037C3C, 1436, "bode_cursors_draw",
   "When byte 0x42E180C8+0xAC60 is set (cursors on), draws the two Bode cursors (x positions = the words +0xAC64 / +0xAC68 from the plot x): dashed 0xBBBBBB vertical lines (2 of every 4 pixels), a 17 x 14 tag with a downward triangle and the labels \"C\" \"1\" / \"C\" \"2\", "
   "then three text rows under the plot (y = plot y + height - 55, 17 apart): \"C1 = \", \"C2 = \" and \"DC = \" (the difference) each followed by the frequency (0xBBBBBB), the gain in dB (0xFF8800, level_label_db) and the phase (0x0088FF, phase_deg_draw), "
   "from the doubles +0xAC70.. +0xACB0 computed by trace_cursor_values. The C1 and C2 rows show the sign, the difference row does not.", BODE + """
static void row(u32 y, const char *lab, u32 sz, double f, double g, double ph, u32 sg)
{
    u32 x = BH(0) + 0xA;
    draw_colour_set(0xBBBBBB);
    half_42e68be2_set(sz);
    text_draw(U16(x), U16(y), lab);
    half_42e68be2_set(2);
    x += TW;
    if (sz != 4)
        x -= 2;
    text_draw(U16(x), U16(y), "= ");
    half_42e68be2_set(2);
    x += TW;
    draw_colour_set(0xBBBBBB);
    frequency_draw(U16(x), U16(y), 0, f);
    x += frequency_draw(U16(x), U16(y), 1, f) + 0x14;
    draw_colour_set(0xFF8800);
    level_label_db(U16(x), U16(y), sg, g);
    x += TW + 0x3C;
    draw_colour_set(0x88FF);
    phase_deg_draw(U16(x), U16(y), sg, ph);
}

static void tag(u32 x, u32 top)
{
    u32 j, y = top + 0xB;
    draw_colour_set(0xBBBBBB);
    rect_fill(U16(x - 8), U16(top), 0x11, 0xE);
    for (j = 0; j < 9; j++) {
        vline_draw(U16(x - j), U16(y), U16(y + 0xD - j));
        vline_draw(U16(x + j), U16(y), U16(y + 0xD - j));
    }
}

void bode_cursors_draw(void)
{
    u32 x1, top, i, x, y;
    int c1, c2, n;
    if (*(volatile unsigned char *)(BB + 0xAC60) == 0)
        return;
    x1 = BH(0) + 1;
    top = BH(2) + 2;
    c1 = BW(0xAC64);
    c2 = BW(0xAC68);
    n = ((int)(BH(2) + BH(6)) - (int)top + 1) / 4;
    word_42e68c74_set(0x42E65418u);
    draw_colour_set(0xBBBBBB);
    if (n != 0) {
        x = (u32)c1 + x1 + 0xFFFF;
        for (i = 0; i < (u32)n; i++)
            vline_draw(U16(x), U16(top + 4 * i), U16(top + 4 * i + 2));
        x = (u32)c2 + x1 + 0xFFFF;
        for (i = 0; i < (u32)n; i++)
            vline_draw(U16(x), U16(top + 4 * i), U16(top + 4 * i + 2));
    }
    x = (u32)c1 + BH(0);
    tag(x, top);
    draw_colour_set(0);
    half_42e68be2_set(2);
    text_draw(U16(x - 3), U16(top), "C");
    text_draw(U16(x - 1), U16(top + 9), "1");
    x = (u32)c2 + BH(0);
    tag(x, top);
    half_42e68be2_set(1);
    draw_colour_set(0);
    text_draw(U16(x - 3), U16(top), "C");
    text_draw(U16(x - 2), U16(top + 9), "2");
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E658D8u);
    y = BH(6) + BH(2) - 0x37;
    row(y, "C1 ", 4, BD(0xAC70), BD(0xAC88), BD(0xACA0), 1);
    row(y + 0x11, "C2 ", 3, BD(0xAC78), BD(0xAC90), BD(0xACA8), 1);
    row(y + 0x22, "DC ", 3, BD(0xAC80), BD(0xAC98), BD(0xACB0), 0);
}
""", src="ui")
