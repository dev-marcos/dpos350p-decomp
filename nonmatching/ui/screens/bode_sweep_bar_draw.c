/*
 * bode_sweep_bar_draw -- stock 0x4002A7F4 (1600 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * After trace_cursor_limits(), draws the sweep-settings bar of the Bode view in the box at layout +0x1F0: a black box, then six buttons 0xA6 x 0x2C every 0xAA from x+3 (records 0x42E61FF8 + 0xB06 + 0xA x n), each with a grey label at (+0xF, +5) and its value at (+0xF, +0x19): 'Amplitude' and 'Offset' (BODE words +0x4C / +0x50 in mV, drawn by voltage_uv_draw), 'Start frequency' and 'Stop frequency' (+0x54 / +0x58, frequency_draw), 'Frequency count' (+0x5C as an integer) and 'Movement' ('Fast move' / 'Slow move' by byte +0x46). Buttons are shaded with toolbar colour +0x1C while pressed.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "state/bode_block.h"
#include "state/ui_windows.h"

#define SYS 0x40544E68u
#define TT (SYS + 0x3DD530)          /* toolbar theme: +0 fill, +4 icon, +8 text, +0x1C pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define BODE 0x42E180C8u             /* frequency-response view block */
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define ENT(n) (TXT + 0x1A0 * (n))

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
    x = WREC(35, x);
    y = WREC(35, y);
    w = WREC(35, w);
    h = WREC(35, h);
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
    func_4002d82c_flag0((tx + 0xF) & 0xFFFF, (y + 0x19) & 0xFFFF, BODES(freq_count), (u32)(S32(BODE + 0x5C) >> 31), 0);
    param_tail(tx, y, 0xB2E);
    tx += 0xAA;
    param_head(tx, y, ENT(77));
    e = BODES(move_slow) ? ENT(66) : ENT(65);
    text_shadow_draw((tx + SB8(e + 0x194) + 0xF) & 0xFFFF, (y + SB8(e + 0x195) + 0x19) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    param_tail(tx, y, 0xB38);
}
