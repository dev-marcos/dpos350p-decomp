/*
 * levels_toolbar_draw -- stock 0x4002AE60 (1460 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the bottom toolbar of the level (spectrum) view in the box at layout +0x21A: a black box, then six buttons every 0xAA from x+3 (records 0x42E61FF8 + 0xB56 + 0xA x n): 'Return', 'Function', 'Screenshot', 'Cursor' (text entries 67, 1, 7, 69) with their icons, an unlabelled icon 0x40085AF4 in 0x0088FF, and the run indicator 0x400857FC drawn red (byte 0x42E68668 set) or green. The last button's background and pressed shade are 0xA7 wide, its touch area 0xA6.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "state/touch_buttons.h"
#include "state/ui_windows.h"
#include "shared/access.h"
#include "state/ui_texts.h"
#include "state/theme.h"

#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records */

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

void levels_toolbar_draw(void)
{
    u32 x = WREC(38, x), y = WREC(38, y), w = WREC(38, w), h = WREC(38, h);
    u32 tx;

    draw_colour_set(0xFFFFFF);
    font_set(0x42E658D8u);
    text_spacing_set(2);
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    tx = x + 3;
    tool_button(tx, y, 0xAA8855, IMGU(40089348), UI_TEXT(67), 0xB56);
    tool_button(tx + 0xAA, y, 0x5588AA, IMGU(40089be8), UI_TEXT(1), 0xB60);
    tool_button(tx + 0xAA * 2, y, 0x88AA55, IMGU(40088c18), UI_TEXT(7), 0xB6A);
    tool_button(tx + 0xAA * 3, y, 0x55AA88, IMGU(400887c8), UI_TEXT(69), 0xB74);

    tx += 0xAA * 4;
    draw_colour_set(THEME_TOOLBAR_FILL);
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(0x88FF);
    draw_text_shadowed((tx + 0x47) & 0xFFFF, (y + 8) & 0xFFFF, IMGU(40085af4), 2, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xB7E));
    if (BREC(294, touch_code)) {
        draw_colour_set(THEME_PRESSED);
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }

    tx += 0xAA;
    draw_colour_set(THEME_TOOLBAR_FILL);
    rect_fill(tx & 0xFFFF, y, 0xA7, 0x2C);
    draw_colour_set(B8(0x42E68668u) ? 0xFF0000 : 0xFF00);
    draw_text_shadowed((tx + 0x47) & 0xFFFF, (y + 8) & 0xFFFF, IMGU(400857fc), 2, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xB88));
    if (BREC(295, touch_code)) {
        draw_colour_set(THEME_PRESSED);
        rect_fill(tx & 0xFFFF, y, 0xA7, 0x2C);
    }
}
