/*
 * hires_menu_draw -- stock 0x40021630 (2036 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the high-resolution mode menu (layout +0xE6..+0xEC): image 0x4008C3FC, the label of text entry 36 ('High resolution mode') followed by the text 0x4008F8CC, then nine option buttons for the byte SYS+0x660: option 0 is 0x19A wide and shows entry 140 ('OFF') above '8 bit'; options 1 .. 8 ('9 bit' .. '16 bit', strings 0x400903D8 + 8 x i) are 0x64 x 0x2D buttons in two rows of four, 0x69 apart. The selected option is filled with theme colour +0x10 and has black text, the others theme +0xC with text colour +0x14. Touch records 0x42E61FF8 + 0x55C + 0xA x i (0x41 high); a pressed option is shaded with theme +8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "sys_info.h"
#include "state/touch_buttons.h"
#include "state/ui_windows.h"
#include "shared/access.h"
#include "state/ui_texts.h"
#include "state/theme.h"

#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records */

/* fill and text colour of option i */
static void option_colours(int i, u32 *txt)
{
    if (SI(hires_option) == i) {
        draw_colour_set(THEME_OPTION_SELECTED);
        *txt = 0;
    } else {
        draw_colour_set(THEME_OPTION);
        *txt = THEME_OPTION_TEXT;
    }
}

void hires_menu_draw(void)
{
    u32 x = WREC(16, x), y = WREC(16, y), w = WREC(16, w), h = WREC(16, h);
    u32 e = UI_TEXT(36), e2 = UI_TEXT(140), r, txt, tw, xo, yb;
    const char *s;
    int i;

    font_set(0x42E658D8u);
    text_spacing_set(1);
    draw_colour_set(THEME_MENU_BG);
    rect_fill(x, y, w, h);
    draw_colour_set(THEME_MENU_TEXT);
    draw_text_shadowed((x + 0x14) & 0xFFFF, (y + 0x17) & 0xFFFF, IMGU(4008c3fc), 2, 2, 0);
    draw_colour_set(THEME_MENU_TEXT);
    text_shadow_draw((x + 0x41 + TEXT_DX(e)) & 0xFFFF, (y + 0x14 + TEXT_DY(e) + 3) & 0xFFFF, TEXT_STR(e), 1, 2, 0);
    tw = text_width(TEXT_STR(e));
    text_shadow_draw((tw + x + TEXT_DX(e) + 0x49) & 0xFFFF, (y + 0x14 + TEXT_DY(e) + 3) & 0xFFFF, (const char *)IMG(4008f8cc), 1, 2, 0);

    r = (y + 0x50) & 0xFFFF;
    option_colours(0, &txt);
    rect_fill((x + 0x14) & 0xFFFF, r, 0x19A, 0x2D);
    draw_colour_set(txt);
    tw = text_width(TEXT_STR(e2));
    text_draw((x + ((0x19A - tw) >> 1) + TEXT_DX(e2) + 0x14) & 0xFFFF, (r + TEXT_DY(e2) + 6) & 0xFFFF, TEXT_STR(e2));
    tw = text_width((const char *)IMG(400903d8));
    text_draw((x + 0x14 + ((0x19A - tw) >> 1)) & 0xFFFF, (r + 0x19) & 0xFFFF, (const char *)IMG(400903d8));
    touch_button_set_rect((x + 0x14) & 0xFFFF, (r - 0xA) & 0xFFFF, 0x19A, 0x41, (volatile unsigned char *)(BTN + 0x55C));
    if (BREC(137, touch_code)) {
        draw_colour_set(THEME_MENU_ROW_PRESSED);
        rect_fill((x + 0x14) & 0xFFFF, r, 0x19A, 0x2D);
    }

    for (i = 1; i <= 8; i++) {
        if (i == 1 || i == 5)
            r += 0x5A;
        yb = (r - 0xA) & 0xFFFF;
        xo = x + 0x14 + 0x69 * ((i - 1) & 3);
        s = (const char *)(IMGU(400903d8) + 8 * i);
        option_colours(i, &txt);
        rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x64, 0x2D);
        draw_colour_set(txt);
        tw = text_width(s);
        text_draw((xo + ((0x64 - tw) >> 1)) & 0xFFFF, (r + 0xF) & 0xFFFF, s);
        touch_button_set_rect(xo & 0xFFFF, yb, 0x64, 0x41, (volatile unsigned char *)(BTN + 0x55C + 0xA * i));
        if (B8(BTN + 0x55D + 0xA * i)) {
            draw_colour_set(THEME_MENU_ROW_PRESSED);
            rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x64, 0x2D);
        }
    }
}
