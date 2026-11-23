/*
 * config_menu_draw -- stock 0x40028740 (884 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the configuration menu (layout +0x68..+0x6E): the box in theme colour SYS+0x3DD550 with a black 2-pixel left edge, then three rows of 0x3C: image 0x4008B2E8 + 0x170 x row, text entry 83 + row ('Save configuration', 'Read configuration', 'Power-on configuration'), icon 0x40019C7C at x+0x124, and touch button 0x42E61FF8 +0x106 / +0x110 / +0x11A. A row whose button is pressed becomes the selection (layout byte +0x67 = row + 1); the selected row is shaded with theme colour +8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "state/ui_windows.h"

#define SYS 0x40544E68u
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define W32(a) (*(volatile u32 *)(a))

void config_menu_draw(void)
{
    u32 x = WREC(7, x), y = WREC(7, y), w = WREC(7, w), h = WREC(7, h);
    u32 x2, sl, e;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    x2 = x + 2;
    for (i = 0; i < 3; i++) {
        sl = y + 0x14 + 0x3C * i;
        e = TXT + 0x1A0 * (83 + i);
        draw_colour_set(W32(TH + 4));
        draw_text_shadowed((x2 + 0x14) & 0xFFFF, sl & 0xFFFF, 0x4008B2E8u + 0x170 * i, 2, 2, 0);
        draw_colour_set(W32(TH + 4));
        text_shadow_draw((x2 + 0x41 + SB8(e + 0x194)) & 0xFFFF, (sl + 3 + SB8(e + 0x195)) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
        icon_19c7c_draw((x2 + 0x122) & 0xFFFF, sl & 0xFFFF);
        touch_button_set_rect(x2 & 0xFFFF, (sl - 0x14) & 0xFFFF, w, 0x3C, (volatile unsigned char *)(BTN + 0x106 + 0xA * i));
        if (B8(BTN + 0x107 + 0xA * i) || WREC(7, f_5) == i + 1) {
            WREC(7, f_5) = i + 1;
            draw_colour_set(W32(TH + 8));
            rect_fill(x2 & 0xFFFF, (sl - 0x14) & 0xFFFF, w, 0x3C);
        }
    }
}
