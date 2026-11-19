# Bottom toolbars of the application views (a row of 0xA6 x 0x2C touch buttons, each with a
# coloured icon and a text-table label).
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define LAY 0x42E64520u              /* UI layout table */
#define TT (SYS + 0x3DD530)          /* toolbar theme: +0 fill, +4 icon, +8 text, +0x1C pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define BODE 0x42E180C8u             /* frequency-response view block */
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define ENT(n) (TXT + 0x1A0 * (n))

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
u32 text_width(const char *s);

/* one toolbar button: background, icon (colour c is set and at once replaced by the theme
   colour, as in the stock code), centred label of text entry e, touch record, pressed shade */
static void tool_button(u32 tx, u32 y, u32 c, u32 img, u32 e, u32 rec)
{
    u32 tw;

    draw_colour_set(W32(TT));
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(c);
    draw_colour_set(W32(TT + 4));
    draw_text_shadowed((tx + 0x49) & 0xFFFF, (y + 2) & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(W32(TT + 8));
    tw = text_width((const char *)W32(e));
    text_shadow_draw((tx + ((0xA6 - tw) >> 1) + SB8(e + 0x194)) & 0xFFFF, (y + SB8(e + 0x195) + 0x19) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }
}
"""

wr(0x4002A184, 1608, "bode_toolbar_draw",
   "Draws the bottom toolbar of the frequency-response (Bode) view in the box at layout +0x1E2: a black box, then six buttons 0xA6 x 0x2C every 0xAA from x+3 with records 0x42E61FF8 "
   "+0xAC0 + 0xA x n: 'Return', 'Calibration', 'Screenshot', 'Cursor' (text entries 67, 68, 7, 69), 'Single' / 'Loop' (entry 70 / 71 by BODE byte +0x44), each with its icon, and "
   "a run indicator icon 0x400857FC drawn red (BODE +0x45 set) or green. Buttons are shaded with toolbar colour +0x1C while pressed.",
   COM + """
void bode_toolbar_draw(void)
{
    u32 x = H16(LAY + 0x1E2), y = H16(LAY + 0x1E4), w = H16(LAY + 0x1E6), h = H16(LAY + 0x1E8);
    u32 tx;

    draw_colour_set(0xFFFFFF);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    tx = x + 3;
    tool_button(tx, y, 0xAA8855, 0x40089348u, ENT(67), 0xAC0);
    tool_button(tx + 0xAA, y, 0x5588AA, 0x40089BE8u, ENT(68), 0xACA);
    tool_button(tx + 0xAA * 2, y, 0x88AA55, 0x40088C18u, ENT(7), 0xAD4);
    tool_button(tx + 0xAA * 3, y, 0x55AA88, 0x400887C8u, ENT(69), 0xADE);

    /* button 4: the label follows the sweep mode, chosen after the text colour is set */
    tx += 0xAA * 4;
    draw_colour_set(W32(TT));
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(0x8855AA);
    draw_colour_set(W32(TT + 4));
    draw_text_shadowed((tx + 0x49) & 0xFFFF, (y + 2) & 0xFFFF, 0x4008BA18u, 2, 2, 0);
    draw_colour_set(W32(TT + 8));
    {
        u32 e = B8(BODE + 0x44) ? ENT(70) : ENT(71);
        u32 tw = text_width((const char *)W32(e));
        text_shadow_draw((tx + ((0xA6 - tw) >> 1) + SB8(e + 0x194)) & 0xFFFF, (y + SB8(e + 0x195) + 0x19) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    }
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xAF2));
    if (B8(BTN + 0xAF3)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }

    /* button 5: run indicator, no label */
    tx += 0xAA;
    draw_colour_set(W32(TT));
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(B8(BODE + 0x45) ? 0xFF0000 : 0xFF00);
    draw_text_shadowed((tx + 0x47) & 0xFFFF, (y + 8) & 0xFFFF, 0x400857FCu, 2, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xAFC));
    if (B8(BTN + 0xAFD)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }
}
""")

wr(0x4002A7F4, 1600, "bode_sweep_bar_draw",
   "After trace_cursor_limits(), draws the sweep-settings bar of the Bode view in the box at layout +0x1F0: a black box, then six buttons 0xA6 x 0x2C every 0xAA from x+3 (records "
   "0x42E61FF8 + 0xB06 + 0xA x n), each with a grey label at (+0xF, +5) and its value at (+0xF, +0x19): 'Amplitude' and 'Offset' (BODE words +0x4C / +0x50 in mV, drawn by "
   "voltage_uv_draw), 'Start frequency' and 'Stop frequency' (+0x54 / +0x58, frequency_draw), 'Frequency count' (+0x5C as an integer) and 'Movement' ('Fast move' / 'Slow move' by "
   "byte +0x46). Buttons are shaded with toolbar colour +0x1C while pressed.",
   COM + """
void trace_cursor_limits(void);
void voltage_uv_draw(u32 x, u32 y, u32 sg, double d);
u32 frequency_draw(u32 x, u32 y, u32 measure, double d);
void func_4002d82c_flag0(u32 x, u32 y, u32 lo, u32 hi, u32 dec);

#define S32(a) (*(volatile int *)(a))

/* background and grey label (text entry e at +0xF + dx, +5 + dy); the value colour is set after */
static void param_head(u32 tx, u32 y, u32 e)
{
    draw_colour_set(W32(TT));
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(0xAAAAAA);
    text_shadow_draw((tx + SB8(e + 0x194) + 0xF) & 0xFFFF, (y + SB8(e + 0x195) + 5) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    draw_colour_set(0xCCCCCC);
}

static void param_tail(u32 tx, u32 y, u32 rec)
{
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }
}

void bode_sweep_bar_draw(void)
{
    u32 x, y, w, h, tx, e, k;

    trace_cursor_limits();
    x = H16(LAY + 0x1F0);
    y = H16(LAY + 0x1F2);
    w = H16(LAY + 0x1F4);
    h = H16(LAY + 0x1F6);
    draw_colour_set(0xFFFFFF);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    tx = x + 3;
    for (k = 0; k < 2; k++) {
        param_head(tx, y, ENT(72 + k));
        voltage_uv_draw((tx + 0xF) & 0xFFFF, (y + 0x19) & 0xFFFF, 0, (double)(int)(1000 * W32(BODE + 0x4C + 4 * k)));
        param_tail(tx, y, 0xB06 + 0xA * k);
        tx += 0xAA;
    }
    for (k = 0; k < 2; k++) {
        param_head(tx, y, ENT(74 + k));
        frequency_draw((tx + 0xF) & 0xFFFF, (y + 0x19) & 0xFFFF, 0, (double)S32(BODE + 0x54 + 4 * k));
        param_tail(tx, y, 0xB1A + 0xA * k);
        tx += 0xAA;
    }
    param_head(tx, y, ENT(76));
    func_4002d82c_flag0((tx + 0xF) & 0xFFFF, (y + 0x19) & 0xFFFF, W32(BODE + 0x5C), (u32)(S32(BODE + 0x5C) >> 31), 0);
    param_tail(tx, y, 0xB2E);
    tx += 0xAA;
    param_head(tx, y, ENT(77));
    e = B8(BODE + 0x46) ? ENT(66) : ENT(65);
    text_shadow_draw((tx + SB8(e + 0x194) + 0xF) & 0xFFFF, (y + SB8(e + 0x195) + 0x19) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    param_tail(tx, y, 0xB38);
}
""")

wr(0x4002AE60, 1460, "levels_toolbar_draw",
   "Draws the bottom toolbar of the level (spectrum) view in the box at layout +0x21A: a black box, then six buttons every 0xAA from x+3 (records 0x42E61FF8 + 0xB56 + 0xA x n): "
   "'Return', 'Function', 'Screenshot', 'Cursor' (text entries 67, 1, 7, 69) with their icons, an unlabelled icon 0x40085AF4 in 0x0088FF, and the run indicator 0x400857FC drawn red "
   "(byte 0x42E68668 set) or green. The last button's background and pressed shade are 0xA7 wide, its touch area 0xA6.",
   COM + """
void levels_toolbar_draw(void)
{
    u32 x = H16(LAY + 0x21A), y = H16(LAY + 0x21C), w = H16(LAY + 0x21E), h = H16(LAY + 0x220);
    u32 tx;

    draw_colour_set(0xFFFFFF);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    tx = x + 3;
    tool_button(tx, y, 0xAA8855, 0x40089348u, ENT(67), 0xB56);
    tool_button(tx + 0xAA, y, 0x5588AA, 0x40089BE8u, ENT(1), 0xB60);
    tool_button(tx + 0xAA * 2, y, 0x88AA55, 0x40088C18u, ENT(7), 0xB6A);
    tool_button(tx + 0xAA * 3, y, 0x55AA88, 0x400887C8u, ENT(69), 0xB74);

    tx += 0xAA * 4;
    draw_colour_set(W32(TT));
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(0x88FF);
    draw_text_shadowed((tx + 0x47) & 0xFFFF, (y + 8) & 0xFFFF, 0x40085AF4u, 2, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xB7E));
    if (B8(BTN + 0xB7F)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }

    tx += 0xAA;
    draw_colour_set(W32(TT));
    rect_fill(tx & 0xFFFF, y, 0xA7, 0x2C);
    draw_colour_set(B8(0x42E68668u) ? 0xFF0000 : 0xFF00);
    draw_text_shadowed((tx + 0x47) & 0xFFFF, (y + 8) & 0xFFFF, 0x400857FCu, 2, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xB88));
    if (B8(BTN + 0xB89)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx & 0xFFFF, y, 0xA7, 0x2C);
    }
}
""")

wr(0x4002B43C, 2000, "levels_param_bar_draw",
   "Draws the settings bar of the level (spectrum) view in the box at layout +0x228: a black box, then seven buttons 0x8E x 0x2C every 0x92 from x+3 (records 0x42E61FF8 + 0xB92 "
   "+ 0xA x n), each with a grey label and its value: 'Reference' and 'Attenuation' (doubles 0x42E68668 +0x148 / +0x140, level_label_dbmv), 'Start frequency', 'Stop frequency', "
   "'Center freq', 'Freq width' (s64 +0x120 / +0x128 / +0x130 / +0x138, frequency_draw) and 'Movement' ('Fast move' / 'Slow move' by byte +3; this last button is 0x8D wide).",
   COM + """
void level_label_dbmv(u32 x, u32 y, u32 c, double v);
u32 frequency_draw(u32 x, u32 y, u32 measure, double d);

#define BLK 0x42E68668u
#define D64(a) (*(volatile double *)(a))
#define S64(a) (*(volatile long long *)(a))

static void lp_head(u32 tx, u32 y, u32 wd, u32 e)
{
    draw_colour_set(W32(TT));
    rect_fill(tx, y, wd, 0x2C);
    draw_colour_set(0xAAAAAA);
    text_shadow_draw((tx + SB8(e + 0x194) + 0xF) & 0xFFFF, (y + SB8(e + 0x195) + 5) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    draw_colour_set(0xCCCCCC);
}

static void lp_tail(u32 tx, u32 y, u32 wd, u32 rec)
{
    touch_button_set_rect(tx, y, wd, 0x2C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx, y, wd, 0x2C);
    }
}

void levels_param_bar_draw(void)
{
    u32 x = H16(LAY + 0x228), y = H16(LAY + 0x22A), w = H16(LAY + 0x22C), h = H16(LAY + 0x22E);
    u32 tx, e, k;

    draw_colour_set(0xFFFFFF);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    tx = (x + 3) & 0xFFFF;
    for (k = 0; k < 2; k++) {
        lp_head(tx, y, 0x8E, ENT(79 + k));
        level_label_dbmv((tx + 0xF) & 0xFFFF, (y + 0x19) & 0xFFFF, 0, D64(BLK + 0x148 - 8 * k));
        lp_tail(tx, y, 0x8E, 0xB92 + 0xA * k);
        tx = (tx + 0x92) & 0xFFFF;
    }
    for (k = 0; k < 4; k++) {
        lp_head(tx, y, 0x8E, ENT(k < 2 ? 74 + k : 79 + k));
        frequency_draw((tx + 0xF) & 0xFFFF, (y + 0x19) & 0xFFFF, 0, (double)S64(BLK + 0x120 + 8 * k));
        lp_tail(tx, y, 0x8E, 0xBA6 + 0xA * k);
        tx = (tx + 0x92) & 0xFFFF;
    }
    lp_head(tx, y, 0x8D, ENT(77));
    e = B8(BLK + 3) ? ENT(66) : ENT(65);
    text_shadow_draw((tx + SB8(e + 0x194) + 0xF) & 0xFFFF, (y + SB8(e + 0x195) + 0x19) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    lp_tail(tx, y, 0x8D, 0xBCE);
}
""")
