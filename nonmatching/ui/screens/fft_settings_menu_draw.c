/*
 * fft_settings_menu_draw -- stock 0x4002BC40 (1744 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the FFT settings menu (layout +0x244..+0x24A). Row 1: image 0x4008BE68, label of text entry 137 ('FFT length') and four option buttons 0x4B x 0x1E at x + 0x14 + 0x4F x i with the centred texts '4K', '8K', '16K', '32K'; the option equal to the word 0x42E6875C is filled with theme colour +0x10 and has black text, the others theme +0xC and text colour +0x14; touch records 0x42E61FF8 + 0xBE2 + 0xA x i, a pressed option is shaded with theme +8. Rows 2 and 3: image, label of entry 138 ('Waterfall_display') / 139 ('3D display') and an ON/OFF switch at x + 0x110 for the bytes 0x42E68669 / 0x42E6866A, with full-width touch records +0xC0A / +0xC14. Row 4: image 0x40089BE8, entry 68 ('Calibration'), the arrow icon at x + 0x126 and record +0xC1E.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "state/levels_block.h"
#include "state/ui_windows.h"
#include "shared/access.h"
#include "state/ui_texts.h"

#define SYS 0x40544E68u
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define BTN 0x42E61FF8u              /* touch button records */

static void label_row(u32 x, u32 y, u32 img, u32 e)
{
    draw_colour_set(W32(TH + 4));
    draw_text_shadowed((x + 0x14) & 0xFFFF, y & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(W32(TH + 4));
    text_shadow_draw((x + 0x41 + TEXT_DX(e)) & 0xFFFF, (y + TEXT_DY(e) + 3) & 0xFFFF, TEXT_STR(e), 1, 2, 0);
}

static void wide_button(u32 x, u32 y, u32 w, u32 rec)
{
    touch_button_set_rect(x, (y - 0x14) & 0xFFFF, w, 0x3C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(W32(TH + 8));
        rect_fill(x, (y - 0x14) & 0xFFFF, w, 0x3C);
    }
}

static const char *len_text(int i)
{
    return i == 0 ? "4K" : i == 1 ? "8K" : i == 2 ? "16K" : "32K";
}

void fft_settings_menu_draw(void)
{
    u32 x = WREC(41, x), y = WREC(41, y), w = WREC(41, w), h = WREC(41, h);
    u32 r, xo, tw;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    draw_colour_set(W32(TH));
    rect_fill((x + 0xA) & 0xFFFF, (y + 4) & 0xFFFF, (w - 0x1E) & 0xFFFF, 0x78);
    label_row(x, y + 0x14, 0x4008BE68u, UI_TEXT(137));
    r = y + 0x50;
    for (i = 0; i < 4; i++) {
        xo = x + 0x14 + 0x4F * i;
        draw_colour_set(LEVELS(spur_mode) == (u32)i ? W32(TH + 0x10) : W32(TH + 0xC));
        rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x4B, 0x1E);
        draw_colour_set(LEVELS(spur_mode) != (u32)i ? W32(TH + 0x14) : 0);
        tw = text_width(len_text(i));
        text_draw((xo + ((0x4B - tw) >> 1)) & 0xFFFF, (r + 8) & 0xFFFF, len_text(i));
        touch_button_set_rect(xo & 0xFFFF, (r - 0xF) & 0xFFFF, 0x4B, 0x3C, (volatile unsigned char *)(BTN + 0xBE2 + 0xA * i));
        if (B8(BTN + 0xBE3 + 0xA * i)) {
            draw_colour_set(W32(TH + 8));
            rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x4B, 0x1E);
        }
    }
    r += 0x48;
    label_row(x, r, 0x4008BFD8u, UI_TEXT(138));
    switch_on_off_draw((x + 0x110) & 0xFFFF, r & 0xFFFF, LEVELS(panel_a_on));
    wide_button(x, r, w, 0xC0A);
    r += 0x3C;
    label_row(x, r, 0x4008C148u, UI_TEXT(139));
    switch_on_off_draw((x + 0x110) & 0xFFFF, r & 0xFFFF, LEVELS(panel_b_on));
    wide_button(x, r, w, 0xC14);
    word_42e68c74_set(0x42E658D8u);
    r += 0x3C;
    label_row(x, r, 0x40089BE8u, UI_TEXT(68));
    icon_19c7c_draw((x + 0x126) & 0xFFFF, r & 0xFFFF);
    wide_button(x, r, w, 0xC1E);
}
