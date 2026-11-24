/*
 * ui_delete_confirm_180 -- stock 0x400271B0 (664 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Dialog 'Delete selected <n> items ?' (n = u16 SYS+0x88BE) in the box at window rectangle 0x42E64520+0x180, with the 'Confirm' button (touch slot 0x42E61FF8+0x9EE, drawn pressed when its byte +1 is set). Colours from SYS+0x3DD54C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "shared/access.h"
#include "state/ui_texts.h"

#define WR16(o) H16(0x42E64520u + (o))
#define BTN ((volatile unsigned char *)0x42E61FF8u)
#define U16(v) ((unsigned short)(v))
#define COL(i) W32(0x40544E68u + 0x3DD54Cu + 4 * (i))

/* "Delete selected" <count SYS+0x88BE> "items" " ?" over a box at rectangle roff of the window
   table 0x42E64520, then a 240 x 50 button labelled "Confirm" (text 95) with touch slot boff. */
static void delete_dialog(u32 roff, u32 boff, int dx_in_x2)
{
    u32 x = WR16(roff), y = WR16(roff + 2), w = WR16(roff + 4), h = WR16(roff + 6);
    u32 cx = x + (w >> 1), ty = y + 0x14, x1, x2, x3, bx, by, tw;
    draw_colour_set(COL(1));
    rect_fill(x, y, w, h);
    draw_colour_set(COL(2));
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E658D8u);
    x1 = U16(x + 0x38);
    text_shadow_draw(U16(x1 + TEXT_DX(UI_TEXT(118))), U16(ty + TEXT_DY(UI_TEXT(118))), TEXT_STR(UI_TEXT(118)), 1, 2, 0);
    x2 = U16(H16(0x42E68BE4u) + x1 + (dx_in_x2 ? TEXT_DX(UI_TEXT(118)) : 0) + 8);
    text_d6f8_shadow_draw(x2, U16(ty), H16(0x40544E68u + 0x88BE), 0, 0, 1, 2, 0);
    x3 = U16(H16(0x42E68BE4u) + x2 + 8);
    text_shadow_draw(U16(x3 + TEXT_DX(UI_TEXT(120))), U16(ty + TEXT_DY(UI_TEXT(120))), TEXT_STR(UI_TEXT(120)), 1, 2, 0);
    text_shadow_draw(U16(H16(0x42E68BE4u) + x3), U16(ty), " ?", 1, 2, 0);
    bx = cx - 0x78;
    by = y + 0x50;
    draw_colour_set(COL(4));
    rect_fill(U16(bx), U16(by), 0xF0, 0x32);
    touch_button_set_rect(U16(bx), U16(by - 10), 0xF0, 0x46, BTN + boff);
    if (BTN[boff + 1]) {
        draw_colour_set(COL(0));
        rect_fill(U16(bx), U16(by), 0xF0, 0x32);
    }
    draw_colour_set(COL(2));
    tw = text_width(TEXT_STR(UI_TEXT(95)));
    text_shadow_draw(U16(x + ((w - tw) >> 1) + TEXT_DX(UI_TEXT(95))), U16(by + TEXT_DY(UI_TEXT(95)) + 0x14), TEXT_STR(UI_TEXT(95)), 1, 2, 0);
}

void ui_delete_confirm_180(void)
{
    delete_dialog(0x180, 0x9EE, 0);
}
