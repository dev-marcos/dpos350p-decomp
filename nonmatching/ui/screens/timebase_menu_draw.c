/*
 * timebase_menu_draw -- stock 0x4001FF74 (5776 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the time-base menu (layout +0xD8..+0xDE): image 0x4008B178 and the label 'Main time base' (text entry 34), or 'Zoom time base' (entry 35) when the split-screen byte SYS+0x2AA is set. The current setting is the byte SYS+0x588 (main) or SYS+0x598 (zoom). Setting 0x2A ('50S') is a 0xA5 x 0x14 button at x + 0x168 in the title row; settings 0x29 .. 0x0C ('20S', '10S', '5S', '2S', '1S', '500mS' .. '5nS' in 1-2-5 steps) are 0x50 x 0x28 buttons in five rows of six, 0x55 apart across and 0x53 down, from (x + 0x14, y + 0x51). The current setting is filled with theme colour +0x10 and has black text, the others theme +0xC with text colour +0x14. Touch records 0x42E61FF8 + 0x426 (setting 0x2A) and + 0x430 + 0xA x (0x29 - setting); a pressed button is shaded with theme +8.
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

static const char tb_text[30][6] = {
    "20S", "10S", "5S", "2S", "1S", "500mS",
    "200mS", "100mS", "50mS", "20mS", "10mS", "5mS",
    "2mS", "1mS", "500uS", "200uS", "100uS", "50uS",
    "20uS", "10uS", "5uS", "2uS", "1uS", "500nS",
    "200nS", "100nS", "50nS", "20nS", "10nS", "5nS",
};

/* fill of a setting button: selected theme +0x10 with black text, else theme +0xC with text colour
   +0x14; returns the text colour */
static u32 fill_colour(u32 sel, u32 v)
{
    if (sel == v) {
        draw_colour_set(THEME_OPTION_SELECTED);
        return 0;
    }
    draw_colour_set(THEME_OPTION);
    return THEME_OPTION_TEXT;
}

void timebase_menu_draw(void)
{
    u32 x = WREC(15, x), y = WREC(15, y), w = WREC(15, w), h = WREC(15, h);
    u32 sel, e, txt, tw, xo, r, rec;
    const char *s;
    int i, j;

    font_set(0x42E658D8u);
    text_spacing_set(1);
    draw_colour_set(THEME_MENU_BG);
    rect_fill(x & 0xFFFF, y, w, h);
    sel = SI(zoom_on) ? SI(zoom_index) : SI(timebase_index);
    draw_colour_set(THEME_MENU_TEXT);
    draw_text_shadowed((x + 0x14) & 0xFFFF, (y + 0x17) & 0xFFFF, IMGU(4008b178), 2, 2, 0);
    draw_colour_set(THEME_MENU_TEXT);
    e = SI(zoom_on) ? UI_TEXT(35) : UI_TEXT(34);
    text_shadow_draw((x + TEXT_DX(e) + 0x41) & 0xFFFF, (y + 0x17 + TEXT_DY(e) + 3) & 0xFFFF, TEXT_STR(e), 1, 2, 0);
    text_spacing_set(2);

    xo = x + 0x168;
    r = y + 0x18;
    txt = fill_colour(sel, 0x2A);
    rect_fill(xo & 0xFFFF, r & 0xFFFF, 0xA5, 0x14);
    draw_colour_set(txt);
    s = "50S";
    tw = text_width(s);
    text_draw((((x + 0x14) & 0xFFFF) + 0x154 + ((0xA5 - tw) >> 1)) & 0xFFFF, (y + 0x1B) & 0xFFFF, s);
    touch_button_set_rect(xo & 0xFFFF, (y + 0xC) & 0xFFFF, 0xA5, 0x2C, (volatile unsigned char *)(BTN + 0x426));
    if (BREC(106, touch_code)) {
        draw_colour_set(THEME_MENU_ROW_PRESSED);
        rect_fill(xo & 0xFFFF, r & 0xFFFF, 0xA5, 0x14);
    }

    r = y + 0x51;
    for (i = 0; i < 5; i++, r += 0x53) {
        for (j = 0; j < 6; j++) {
            int k = 6 * i + j;
            xo = x + 0x14 + 0x55 * j;
            rec = 0x430 + 0xA * k;
            txt = fill_colour(sel, 0x29 - k);
            rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x50, 0x28);
            draw_colour_set(txt);
            tw = text_width(tb_text[k]);
            text_draw((((0x50 - tw) >> 1) + (xo & 0xFFFF)) & 0xFFFF, (r + 0xC) & 0xFFFF, tb_text[k]);
            touch_button_set_rect(xo & 0xFFFF, (r - 0xF) & 0xFFFF, 0x50, 0x46, (volatile unsigned char *)(BTN + rec));
            if (B8(BTN + rec + 1)) {
                draw_colour_set(THEME_MENU_ROW_PRESSED);
                rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x50, 0x28);
            }
        }
    }
}
