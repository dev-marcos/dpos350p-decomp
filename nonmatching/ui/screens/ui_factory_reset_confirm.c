/*
 * ui_factory_reset_confirm -- stock 0x4002967C (668 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Message box 'Notice !' / 'Restore to factory' 'settings ?' (texts 91 and 102..104 of the UI table) in the box at window rectangle 0x42E64520+0x1AA, with the 'Confirm' button (text 95, touch slot 0x42E61FF8+0x1EC, drawn pressed when its byte +1 is set). Colours from SYS+0x3DD550.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "shared/access.h"
#include "state/ui_texts.h"
#include "state/theme.h"

#define WR16(o) H16(0x42E64520u + (o))
#define BTN ((volatile unsigned char *)0x42E61FF8u)
#define U16(v) ((unsigned short)(v))

/* Message box at rectangle roff: title text t centred, three lines b..b+2 at x offset xo, and a
   'Confirm' button (text 95) bd pixels lower than usual, touch slot boff. */
static void msgbox(u32 roff, u32 t, u32 b, u32 xo, u32 bd, u32 boff)
{
    u32 x = WR16(roff), y = WR16(roff + 2), w = WR16(roff + 4), h = WR16(roff + 6);
    u32 iw, x2, y2, bx, bw, by, tw;
    font_set(0x42E658D8u);
    text_spacing_set(2);
    draw_colour_set(THEME_MENU_BG);
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    iw = U16(w - 2);
    x2 = x + 2;
    y2 = y + 0x14;
    draw_colour_set(THEME_MENU_TEXT);
    tw = text_width(TEXT_STR(UI_TEXT(t)));
    text_shadow_draw(U16(x2 + ((iw - tw) >> 1) + TEXT_DX(UI_TEXT(t))), U16(y2 + TEXT_DY(UI_TEXT(t)) + 3), TEXT_STR(UI_TEXT(t)), 1, 2, 0);
    text_shadow_draw(U16(x2 + TEXT_DX(UI_TEXT(b)) + xo), U16(y2 + TEXT_DY(UI_TEXT(b)) + 0x23), TEXT_STR(UI_TEXT(b)), 1, 2, 0);
    text_shadow_draw(U16(x2 + TEXT_DX(UI_TEXT(b + 1)) + xo), U16(y2 + TEXT_DY(UI_TEXT(b + 1)) + 0x39), TEXT_STR(UI_TEXT(b + 1)), 1, 2, 0);
    text_shadow_draw(U16(x2 + TEXT_DX(UI_TEXT(b + 2)) + xo), U16(y2 + TEXT_DY(UI_TEXT(b + 2)) + 0x4F), TEXT_STR(UI_TEXT(b + 2)), 1, 2, 0);
    bx = x2 + 0x14;
    bw = U16(iw - 0x28);
    by = y2 + 0x6E + bd;
    draw_colour_set(THEME_OPTION);
    rect_fill(U16(bx), U16(by), bw, 0x28);
    draw_colour_set(THEME_MENU_TEXT);
    tw = text_width(TEXT_STR(UI_TEXT(95)));
    text_shadow_draw(U16(x2 + ((iw - tw) >> 1) + TEXT_DX(UI_TEXT(95))), U16(y2 + TEXT_DY(UI_TEXT(95)) + 0x7A + bd), TEXT_STR(UI_TEXT(95)), 1, 2, 0);
    touch_button_set_rect(U16(bx), U16(y2 + 0x64 + bd), bw, 0x3C, BTN + boff);
    if (BTN[boff + 1]) {
        draw_colour_set(THEME_MENU_ROW_PRESSED);
        rect_fill(U16(bx), U16(by), bw, 0x28);
    }
}

void ui_factory_reset_confirm(void)
{
    msgbox(0x1AA, 91, 102, 0x31, 0, 0x1EC);
}
