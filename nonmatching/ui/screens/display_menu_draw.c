/*
 * display_menu_draw -- stock 0x4001CE30 (1916 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the display settings menu (layout +0x92..+0x98), seven rows with an image at x + 0x14 and the label of text entry 16 + row. Row 0 'Waveform light': '%' at the right, the value byte SYS+0x654 as a number (0x4002D724) left of it and a slider 0x40019CF8 for it one row lower, touch record 0x42E61FF8 + 0x20A. Rows 1 .. 3 'Color temperature', 'X-Y mode display', 'Background gird': ON/OFF switches at x + 0x172 for the bytes SYS+0x653, SYS+0x652, SYS+0x655 (grid), records +0x214 / +0x21E / +0x228. Rows 4 .. 6 'Capture output', 'Frequency response analyzer', 'Spectrum analyzer': the arrow icon at x + 0x186, records +0x232 / +0x23C / +0x246; row 6 also shows ' (CH1)' after its label. A pressed row (rows 1 .. 6) is shaded with theme colour +8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "sys_info.h"
#include "state/ui_windows.h"
#include "shared/access.h"
#include "state/ui_texts.h"
#include "state/theme.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
u32 number_text_width_mode0_raw(u32 a, u32 b, u32 c) __asm__("number_text_width_mode0");

#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records */

static void label_row(u32 x, u32 y, u32 img, u32 e)
{
    draw_colour_set(THEME_MENU_TEXT);
    draw_text_shadowed((x + 0x14) & 0xFFFF, y & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(THEME_MENU_TEXT);
    text_shadow_draw((x + 0x41 + TEXT_DX(e)) & 0xFFFF, (y + TEXT_DY(e) + 3) & 0xFFFF, TEXT_STR(e), 1, 2, 0);
}

static void wide_button(u32 x, u32 y, u32 w, u32 rec)
{
    touch_button_set_rect(x, (y - 0x14) & 0xFFFF, w, 0x3C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(THEME_MENU_ROW_PRESSED);
        rect_fill(x, (y - 0x14) & 0xFFFF, w, 0x3C);
    }
}

void display_menu_draw(void)
{
    u32 x = WREC(10, x), y = WREC(10, y), w = WREC(10, w), h = WREC(10, h);
    u32 r0 = y + 0x14, r, nw;

    font_set(0x42E658D8u);
    text_spacing_set(1);
    draw_colour_set(THEME_MENU_BG);
    rect_fill(x, y, w, h);
    label_row(x, r0, IMGU(40089628), UI_TEXT(16));
    text_shadow_draw((x + w - 0x1C) & 0xFFFF, (r0 + 3) & 0xFFFF, "%", 1, 2, 0);
    text_spacing_set(3);
    nw = number_text_width_mode0_raw(SI(waveform_light), 0, 0);
    number_mode0_shadow_draw((x + w - 0x20 - nw) & 0xFFFF, (r0 + 3) & 0xFFFF, SI(waveform_light), 0, 0, 1, 2, 0);
    text_spacing_set(1);
    slider_draw((x + 0x14) & 0xFFFF, (r0 + 0x3C) & 0xFFFF, SI(waveform_light));
    touch_button_set_rect(x, (r0 + 0x3C - 0x16) & 0xFFFF, w, 0x3C, (volatile unsigned char *)(BTN + 0x20A));

    r = (r0 + 0x3C + 0x3D) & 0xFFFF;
    label_row(x, r, IMGU(40089798), UI_TEXT(17));
    switch_on_off_draw((x + 0x172) & 0xFFFF, (r - 5) & 0xFFFF, SI(palette_alt));
    wide_button(x, r, w, 0x214);
    r = (r + 0x3C) & 0xFFFF;
    label_row(x, r, IMGU(40089908), UI_TEXT(18));
    switch_on_off_draw((x + 0x172) & 0xFFFF, (r - 5) & 0xFFFF, SI(f_652));
    wide_button(x, r, w, 0x21E);
    r = (r + 0x3C) & 0xFFFF;
    label_row(x, r, IMGU(40089a78), UI_TEXT(19));
    switch_on_off_draw((x + 0x172) & 0xFFFF, (r - 5) & 0xFFFF, SI(grid_on));
    wide_button(x, r, w, 0x228);

    r += 0x3C;
    label_row(x, r, IMGU(40089d58), UI_TEXT(20));
    icon_19c7c_draw((x + 0x186) & 0xFFFF, r & 0xFFFF);
    wide_button(x, r, w, 0x232);
    r += 0x3C;
    label_row(x, r, IMGU(4008bb88), UI_TEXT(21));
    icon_19c7c_draw((x + 0x186) & 0xFFFF, r & 0xFFFF);
    wide_button(x, r, w, 0x23C);
    r += 0x3C;
    label_row(x, r, IMGU(4008bcf8), UI_TEXT(22));
    text_spacing_set(2);
    text_shadow_draw((H16(0x42E68BE4u) + x + 0x41) & 0xFFFF, (r + 3) & 0xFFFF, (const char *)IMG(40090318), 1, 2, 0);
    text_spacing_set(1);
    icon_19c7c_draw((x + 0x186) & 0xFFFF, r & 0xFFFF);
    wide_button(x, r, w, 0x246);
}
