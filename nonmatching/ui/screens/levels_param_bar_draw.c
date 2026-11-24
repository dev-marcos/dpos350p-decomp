/*
 * levels_param_bar_draw -- stock 0x4002B43C (2000 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the settings bar of the level (spectrum) view in the box at layout +0x228: a black box, then seven buttons 0x8E x 0x2C every 0x92 from x+3 (records 0x42E61FF8 + 0xB92 + 0xA x n), each with a grey label and its value: 'Reference' and 'Attenuation' (doubles 0x42E68668 +0x148 / +0x140, level_label_dbmv), 'Start frequency', 'Stop frequency', 'Center freq', 'Freq width' (s64 +0x120 / +0x128 / +0x130 / +0x138, frequency_draw) and 'Movement' ('Fast move' / 'Slow move' by byte +3; this last button is 0x8D wide).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "state/levels_block.h"
#include "state/ui_windows.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define TT (SYS + 0x3DD530)          /* toolbar theme: +0 fill, +4 icon, +8 text, +0x1C pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
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

#define BLK 0x42E68668u

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
    u32 x = WREC(39, x), y = WREC(39, y), w = WREC(39, w), h = WREC(39, h);
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
    e = LEVELS(move_slow) ? ENT(66) : ENT(65);
    text_shadow_draw((tx + SB8(e + 0x194) + 0xF) & 0xFFFF, (y + SB8(e + 0x195) + 0x19) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    lp_tail(tx, y, 0x8D, 0xBCE);
}
