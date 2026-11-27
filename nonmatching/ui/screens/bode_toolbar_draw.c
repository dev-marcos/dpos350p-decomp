/*
 * bode_toolbar_draw -- stock 0x4002A184 (1608 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the bottom toolbar of the frequency-response (Bode) view in the box at layout +0x1E2: a black box, then six buttons 0xA6 x 0x2C every 0xAA from x+3 with records 0x42E61FF8 +0xAC0 + 0xA x n: 'Return', 'Calibration', 'Screenshot', 'Cursor' (text entries 67, 68, 7, 69), 'Single' / 'Loop' (entry 70 / 71 by BODE byte +0x44), each with its icon, and a run indicator icon 0x400857FC drawn red (BODE +0x45 set) or green. Buttons are shaded with toolbar colour +0x1C while pressed.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "state/bode_block.h"
#include "state/touch_buttons.h"
#include "state/ui_windows.h"
#include "shared/access.h"
#include "state/ui_texts.h"
#include "state/theme.h"

#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records */
#define BODE 0x42E180C8u             /* frequency-response view block */

/* one toolbar button: background, icon (colour c is set and at once replaced by the theme
   colour, as in the stock code), centred label of text entry e, touch record, pressed shade */
static void tool_button(u32 tx, u32 y, u32 c, u32 img, u32 e, u32 rec)
{
    u32 tw;

    draw_colour_set(THEME_TOOLBAR_FILL);
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(c);
    draw_colour_set(THEME_TOOLBAR_ICON);
    draw_text_shadowed((tx + 0x49) & 0xFFFF, (y + 2) & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(THEME_TOOLBAR_TEXT);
    tw = text_width(TEXT_STR(e));
    text_shadow_draw((tx + ((0xA6 - tw) >> 1) + TEXT_DX(e)) & 0xFFFF, (y + TEXT_DY(e) + 0x19) & 0xFFFF, TEXT_STR(e), 1, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(THEME_PRESSED);
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }
}

void bode_toolbar_draw(void)
{
    u32 x = WREC(34, x), y = WREC(34, y), w = WREC(34, w), h = WREC(34, h);
    u32 tx;

    draw_colour_set(0xFFFFFF);
    font_set(0x42E658D8u);
    text_spacing_set(2);
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    tx = x + 3;
    tool_button(tx, y, 0xAA8855, IMGU(40089348), UI_TEXT(67), 0xAC0);
    tool_button(tx + 0xAA, y, 0x5588AA, IMGU(40089be8), UI_TEXT(68), 0xACA);
    tool_button(tx + 0xAA * 2, y, 0x88AA55, IMGU(40088c18), UI_TEXT(7), 0xAD4);
    tool_button(tx + 0xAA * 3, y, 0x55AA88, IMGU(400887c8), UI_TEXT(69), 0xADE);

    /* button 4: the label follows the sweep mode, chosen after the text colour is set */
    tx += 0xAA * 4;
    draw_colour_set(THEME_TOOLBAR_FILL);
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(0x8855AA);
    draw_colour_set(THEME_TOOLBAR_ICON);
    draw_text_shadowed((tx + 0x49) & 0xFFFF, (y + 2) & 0xFFFF, IMGU(4008ba18), 2, 2, 0);
    draw_colour_set(THEME_TOOLBAR_TEXT);
    {
        u32 e = BODES(loop) ? UI_TEXT(70) : UI_TEXT(71);
        u32 tw = text_width(TEXT_STR(e));
        text_shadow_draw((tx + ((0xA6 - tw) >> 1) + TEXT_DX(e)) & 0xFFFF, (y + TEXT_DY(e) + 0x19) & 0xFFFF, TEXT_STR(e), 1, 2, 0);
    }
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xAF2));
    if (BREC(280, touch_code)) {
        draw_colour_set(THEME_PRESSED);
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }

    /* button 5: run indicator, no label */
    tx += 0xAA;
    draw_colour_set(THEME_TOOLBAR_FILL);
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(BODES(running) ? 0xFF0000 : 0xFF00);
    draw_text_shadowed((tx + 0x47) & 0xFFFF, (y + 8) & 0xFFFF, IMGU(400857fc), 2, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xAFC));
    if (BREC(281, touch_code)) {
        draw_colour_set(THEME_PRESSED);
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }
}
