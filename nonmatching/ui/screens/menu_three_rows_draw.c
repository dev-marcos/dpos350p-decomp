/*
 * menu_three_rows_draw -- stock 0x4001E8E0 (764 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the three-row menu box of layout 0x42E64520 +0xAE..+0xB4: fills it with theme colour SYS+0x3DD550, then for each row (0x3C high) the image 0x4008A038 / 0x4008A1A8 / 0x4008A318, the label of text-table entry 110 + row (with its per-language x / y offsets), the icon 0x40019C7C at the right, and a touch button (records 0x42E61FF8 +0x340 / +0x34A / +0x354) that is shaded with theme colour +8 while pressed.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "state/ui_windows.h"
#include "shared/access.h"
#include "state/ui_texts.h"
#include "state/theme.h"

#define SYS 0x40544E68u

void menu_three_rows_draw(void)
{
    u32 x = WREC(12, x), y = WREC(12, y);
    u32 w = WREC(12, w), h = WREC(12, h);
    u32 sl, e;
    volatile unsigned char *rec;
    int i;

    font_set(0x42E658D8u);
    text_spacing_set(1);
    draw_colour_set(THEME_MENU_BG);
    rect_fill(x, y, w, h);
    for (i = 0; i < 3; i++) {
        sl = y + 0x14 + 0x3C * i;
        e = UI_TEXT(110 + i);
        rec = (volatile unsigned char *)(0x42E61FF8u + 0x340 + 0xA * i);
        draw_colour_set(THEME_MENU_TEXT);
        draw_text_shadowed((x + 0x14) & 0xFFFF, sl & 0xFFFF, IMGU(4008a038) + 0x170 * i, 2, 2, 0);
        draw_colour_set(THEME_MENU_TEXT);
        text_shadow_draw((x + 0x41 + TEXT_DX(e)) & 0xFFFF, (sl + 3 + TEXT_DY(e)) & 0xFFFF, TEXT_STR(e), 1, 2, 0);
        icon_19c7c_draw((x + 0x14A) & 0xFFFF, sl & 0xFFFF);
        touch_button_set_rect(x, (sl - 0x14) & 0xFFFF, w, 0x3C, rec);
        if (rec[1]) {
            draw_colour_set(THEME_MENU_ROW_PRESSED);
            rect_fill(x, (sl - 0x14) & 0xFFFF, w, 0x3C);
        }
    }
}
