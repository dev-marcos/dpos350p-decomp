/*
 * ch1_menu_draw -- stock 0x4001EBE4 (2444 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the CH1 settings menu (layout +0xBC..): rows 'Channel enable' (text entry 25, ON/OFF switch for SYS+0x3E), 'Proble attenuation' (entry 27, options '1X' / '10X' / '100X' for SYS+0x40), 'Coupling mode' (entry 28, options 'DC' / 'AC' for SYS+0x3F), 'FFT display' (entry 31, switch for SYS+0x42) and 'Bandwidth limit' (entry 32, options 'Full 350M' / '150M' / '20M' for SYS+0x47). The selected option is filled with theme colour +0x10 and has black text; touch records 0x42E61FF8 + 0x35E onwards, a pressed row or option is shaded with theme +8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "shared/access.h"
#include "state/ui_texts.h"
#include "state/theme.h"

#define SYS 0x40544E68u
#define LAY 0x42E64520u              /* UI layout table */
#define BTN 0x42E61FF8u              /* touch button records */

static void label_row(u32 x, u32 y, u32 img, u32 e)
{
    draw_colour_set(THEME_MENU_TEXT);
    draw_text_shadowed((x + 0x14) & 0xFFFF, y & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(THEME_MENU_TEXT);
    text_shadow_draw((x + 0x41 + TEXT_DX(e)) & 0xFFFF, (y + TEXT_DY(e) + 3) & 0xFFFF, TEXT_STR(e), 1, 2, 0);
}

/* row with an ON/OFF switch at x + 0x172 and a full-width touch record */
static void switch_row(u32 x, u32 y, u32 w, u32 on, u32 rec)
{
    switch_on_off_draw((x + 0x172) & 0xFFFF, (y - 5) & 0xFFFF, on);
    touch_button_set_rect(x, (y - 0x14) & 0xFFFF, w, 0x3C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(THEME_MENU_ROW_PRESSED);
        rect_fill(x, (y - 0x14) & 0xFFFF, w, 0x3C);
    }
}

/* option button k of a row: fill (selected: theme +0x10 with black text, else theme +0xC with
   text colour +0x14), returns the text colour after setting the fill */
static u32 option_fill(u32 sel, int k, u32 xo, u32 y, u32 wd)
{
    u32 txt;

    if (sel == (u32)k) {
        draw_colour_set(THEME_OPTION_SELECTED);
        txt = 0;
    } else {
        draw_colour_set(THEME_OPTION);
        txt = THEME_OPTION_TEXT;
    }
    rect_fill(xo & 0xFFFF, y & 0xFFFF, wd, 0x1E);
    return txt;
}

static void option_button(u32 xo, u32 y, u32 wd, u32 rec)
{
    touch_button_set_rect(xo & 0xFFFF, (y - 0xF) & 0xFFFF, wd, 0x3C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(THEME_MENU_ROW_PRESSED);
        rect_fill(xo & 0xFFFF, y & 0xFFFF, wd, 0x1E);
    }
}

/* lay: layout offset of the box, so: offset of the channel's settings in SYS, ro: offset of the
   channel's touch records, en: text entry of the 'Channel enable' label */
static void channel_menu(u32 lay, u32 so, u32 ro, u32 en)
{
    u32 x = H16(LAY + lay), y = H16(LAY + lay + 2), w = H16(LAY + lay + 4), h = H16(LAY + lay + 6);
    u32 r, xo, tw, e;
    const char *s;
    int k;

    font_set(0x42E658D8u);
    text_spacing_set(1);
    draw_colour_set(THEME_MENU_BG);
    rect_fill(x, y, w, h);

    r = y + 0x19;
    label_row(x, r, 0x4008A488u, UI_TEXT(en));
    switch_row(x, r, w, B8(SYS + so + 0x3E), ro + 0x35E);

    r += 0x39;
    label_row(x, r, 0x4008A5F8u, UI_TEXT(27));
    r += 0x39;
    for (k = 0; k < 3; k++) {
        xo = x + 0x14 + 0x8A * k;
        draw_colour_set(option_fill(B8(SYS + so + 0x40), k, xo, r, 0x86));
        s = k == 0 ? "1X" : k == 1 ? "10X" : (const char *)0x400902C8u;
        tw = text_width(s);
        text_draw((xo + ((0x86 - tw) >> 1)) & 0xFFFF, (r + 8) & 0xFFFF, s);
        option_button(xo, r, 0x86, ro + 0x368 + 0xA * k);
    }

    r += 0x43;
    label_row(x, r, 0x4008A768u, UI_TEXT(28));
    r += 0x39;
    for (k = 0; k < 2; k++) {
        xo = x + 0x14 + 0xCF * k;
        e = UI_TEXT(29 + k);
        draw_colour_set(option_fill(B8(SYS + so + 0x3F), k, xo, r, 0xCB));
        tw = text_width(TEXT_STR(e));
        text_draw((x + ((0xCB - tw) >> 1) + TEXT_DX(e) + 0x14 + 0xCF * k) & 0xFFFF, (r + TEXT_DY(e) + 8) & 0xFFFF, TEXT_STR(e));
        option_button(xo, r, 0xCB, ro + 0x386 + 0xA * k);
    }

    r += 0x43;
    label_row(x, r, 0x4008A8D8u, UI_TEXT(31));
    switch_row(x, r, w, B8(SYS + so + 0x42), ro + 0x39A);

    r += 0x39;
    label_row(x, r, 0x4008AA48u, UI_TEXT(32));
    r += 0x39;
    for (k = 0; k < 3; k++) {
        xo = x + 0x14 + 0x8A * k;
        draw_colour_set(option_fill(B8(SYS + so + 0x47), k, xo, r, 0x86));
        if (k == 0) {
            e = UI_TEXT(33);
            tw = text_width(TEXT_STR(e));
            text_draw((x + ((0x86 - tw) >> 1) + TEXT_DX(e) + 0x14) & 0xFFFF, (r + TEXT_DY(e) + 8) & 0xFFFF, TEXT_STR(e));
        } else {
            s = k == 1 ? (const char *)0x40090340u : "20M";
            tw = text_width(s);
            text_draw((xo + ((0x86 - tw) >> 1)) & 0xFFFF, (r + 8) & 0xFFFF, s);
        }
        option_button(xo, r, 0x86, ro + (k == 0 ? 0x3B8 : 0x39A + 0xA * k));
    }
}

void ch1_menu_draw(void)
{
    channel_menu(0xBC, 0, 0, 25);
}
